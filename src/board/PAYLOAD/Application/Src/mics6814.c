#include "mics6814.h"

#include <math.h>
#include <errno.h>

#include <stm32f1xx_hal.h>

#include <its-time.h>

#include "analog.h"

/*
	Будем делать вот как. В даташите есть красивые графики,
	показывающие зависимость r/r0 от ppm для разных газов
	Всякие левые газы мы просто не будет учитывать и для каждого сенсора возьмем только "титульный" газ

	Будем искать функцию зависимости ppm(r/r0) через график.
	Обозначим ppm как y, а r/r0 как х. На графиках наоборот, но нам нужна обратная зависимость.

	График линейный в осях log-log а значит описывается выражением log(y) = a*log(x) + b.
	Решаем вот такую систему уравнений относительно a и b
	log(y1) = a*log(x1) + b
	log(y2) = a*log(x2) + b

	Получается:
	a = (log(y1) - log(y2)) / (log(x1) - log(x2))
	b = log(y2) - a * log(x2)

	Теперь у нас есть выражение: log(y) = a*log(x) + b, в котором мы знаем и a и b.
	Экспоненцируем обе части уравнения и получаем
	y = x**a * e**b.

	// Таким подходом определяем зависимости для всех трех сеносоров
*/

// О подключении датчика
/*
 Каждый сенсор датчика подключен по следующей схеме:

 vcc 3.3 +--====--+--====--+--====--+--====-- gnd
         |   r1   |   r2   |   r3   |   rx
         |       /        /         |
         |        |        |        |
         +--------+--------+        +---- к АЦП

  r1, r2 и r3 это известные нам балансирующие резисторы.
  rx - незивестное сопротивление, которое выдает сенсор.
  Поскольку rx может меняться очень сильно, а мы хотим знать его довольно точно,
  мы сделали переменное плечо делителя, с которого снимаем напряжение посредством АЦП.
  Это плечо может состять только из r3, или из r3 + r2 или из r3 + r2 + r1*/


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Сенсор CO (RED)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Коэфициенты пересчёта в ppm
/* Посчитаны по графику в даташите из двух точек:
 * x1 = 0.01, y1 = 1000
 * x2 = 0.4, y2 = 10 */
#define MICS6814_CO_COEFFS_A	(-1.2483927011635698)
#define MICS6814_CO_COEFFS_EXP_B	(3.1857713161420667)

// Значение R0 (сопротивление сенсора в калибровочной атмосфере)
#define MICS6814_CO_R0	(241*1000)
// Значение резистора в верхнем плече делителя сенсора
#define MICS6814_CO_RB	(189*1000)

// Второй порт/пин для управления балансирующими резисторами
#define MICS6814_CO_CTRL_2_PORT		GPIOB
#define MICS6814_CO_CTRL_2_PIN		GPIO_PIN_2

// Первый порт/пин для управления балансирующими резисторами
#define MICS6814_CO_CTRL_1_PORT		GPIOB
#define MICS6814_CO_CTRL_1_PIN		GPIO_PIN_1

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Сенсор NO2 (OX)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Коэфициенты пересчёта в ppm
/* Посчитаны по графику в даташите из двух точек:
 * x1 = 20, y1 = 3
 * x2 = 0.3, y2 = 0.05 */
#define MICS6814_NO2_COEFFS_A	(0.9749124012986611)
#define MICS6814_NO2_COEFFS_EXP_B	(0.16170780328191597)

// Значение R0 (сопротивление сенсора в калибровочной атмосфере)
#define MICS6814_NO2_R0	(7.18*1000)
// Значение резистора в верхнем плече делителя сенсора
#define MICS6814_NO2_RB	(4.63*1000)

// Второй порт/пин для управления балансирующими резисторами
#define MICS6814_NO2_CTRL_2_PORT	GPIOA
#define MICS6814_NO2_CTRL_2_PIN		GPIO_PIN_4

// Номинал Балансирующего резистора R2 из резисторного делителя
#define MICS6814_NO2_R2_VALUE	(10*1000)

// Первый порт/пин для управления балансирующими резисторами
#define MICS6814_NO2_CTRL_1_PORT	GPIOA
#define MICS6814_NO2_CTRL_1_PIN		GPIO_PIN_5

// Номинал Балансирующего резистора R3 из резисторного делителя
#define MICS6814_NO2_R3_VALUE	(10*1000)

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Сенсор NH3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Коэфициенты пересчёта в ppm
/* Посчитаны по графику в даташите из двух точек:
 * x1 = 0.8, y1 = 1
 * x2 = 0.08, y2 = 70*/
#define MICS6814_NH3_COEFFS_A	(-1.845098040014257)
#define MICS6814_NH3_COEFFS_EXP_B	(0.6625086072228469)

// Значение R0 (сопротивление сенсора в калибровочной атмосфере)
#define MICS6814_NH3_R0	(23.02*1000)
// Значение резистора в верхнем плече делителя сенсора
#define MICS6814_NH3_RB	(18.78*1000)

// Второй порт/пин для управления балансирующими резисторами
#define MICS6814_NH3_CTRL_2_PORT	GPIOA
#define MICS6814_NH3_CTRL_2_PIN		GPIO_PIN_6

// Первый порт/пин для управления балансирующими резисторами
#define MICS6814_NH3_CTRL_1_PORT	GPIOA
#define MICS6814_NH3_CTRL_1_PIN		GPIO_PIN_7


//! С каким именно сенсором мы работаем
typedef enum mics6814_sensor_t {
	MICS6814_SENSOR_CO,
	MICS6814_SENSOR_NO2,
	MICS6814_SENSOR_NH3,
} mics6814_sensor_t;


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Управление верхним плечем делителя
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//! Количество резисторов в верхнем плече резисторного делителя сенсоров
typedef enum mics6814_divider_upper_half_mode_t {
	//! Используются три резистора в плече
	MICS6814_DIVIER_UPPER_HALF_TRIPLE,
	//! Используются два резистора в плече
	MICS6814_DIVIER_UPPER_HALF_DOUBLE,
	//! Используется один резистор в плече
	MICS6814_DIVIER_UPPER_HALF_SINGLE,
} mics6814_divider_upper_half_mode_t;


// Перевод пина в input и highz состояние
static void _pin_to_input_mode(GPIO_TypeDef * port, uint32_t pin)
{
	GPIO_InitTypeDef init;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_INPUT;
	init.Pin = pin;

	HAL_GPIO_Init(port, &init);
}


// Перевод пина в output состояние с высоким логическим уровнем
static void _pin_to_high_output(GPIO_TypeDef * port, uint32_t pin)
{
	GPIO_InitTypeDef init;
	init.Pull = GPIO_NOPULL;
	init.Speed = GPIO_SPEED_FREQ_LOW;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Pin = pin;

	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	HAL_GPIO_Init(port, &init);
}


// Устанавливаем верхнее плечо делителя для указанного сенсора
int _divider_upper_half_set_mode(mics6814_sensor_t target, mics6814_divider_upper_half_mode_t mode)
{
	int error;
	GPIO_TypeDef * port1, * port2;
	uint32_t pin1, pin2;

	switch (target)
	{
	case MICS6814_SENSOR_CO:
		port1 = MICS6814_CO_CTRL_1_PORT;
		pin1 = MICS6814_CO_CTRL_1_PIN;
		port2 = MICS6814_CO_CTRL_2_PORT;
		pin2 = MICS6814_CO_CTRL_2_PIN;
		break;

	case MICS6814_SENSOR_NO2:
		port1 = MICS6814_NO2_CTRL_1_PORT;
		pin1 = MICS6814_NO2_CTRL_1_PIN;
		port2 = MICS6814_NO2_CTRL_2_PORT;
		pin2 = MICS6814_NO2_CTRL_2_PIN;
		break;

	case MICS6814_SENSOR_NH3:
		port1 = MICS6814_NH3_CTRL_1_PORT;
		pin1 = MICS6814_NH3_CTRL_1_PIN;
		port2 = MICS6814_NH3_CTRL_2_PORT;
		pin2 = MICS6814_NH3_CTRL_2_PIN;
		break;

	default:
		error = -ENOSYS;
		break;
	};

	if (0 != error)
		return error;

	switch (mode)
	{
	case MICS6814_DIVIER_UPPER_HALF_TRIPLE:
		_pin_to_input_mode(port1, pin1);
		_pin_to_input_mode(port2, pin2);
		break;

	case MICS6814_DIVIER_UPPER_HALF_DOUBLE:
		_pin_to_input_mode(port1, pin1);
		_pin_to_high_output(port2, pin2);
		break;

	case MICS6814_DIVIER_UPPER_HALF_SINGLE:
		_pin_to_high_output(port1, pin1);
		_pin_to_high_output(port2, pin2);
		break;

	default:
		error = -EINVAL;
	}

	if (0 != error)
		return error;

	return 0;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Работа с сенсором по-крупному
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


int mics6814_init()
{
	// Нужно настроить GPIO для упрваления плечами
	// но они правильно настроены с резета (в in)
	// поэтому мы их не трогаем тоже
	// Только включим клоки (хотя они и так должны быть включены)
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// Настриваем все резисторы
	_divider_upper_half_set_mode(MICS6814_SENSOR_CO,  MICS6814_DIVIER_UPPER_HALF_TRIPLE);
	_divider_upper_half_set_mode(MICS6814_SENSOR_NO2, MICS6814_DIVIER_UPPER_HALF_TRIPLE);
	_divider_upper_half_set_mode(MICS6814_SENSOR_NH3, MICS6814_DIVIER_UPPER_HALF_TRIPLE);

	return 0;
}


// Делает замер для одного из сенсоров датчика
/* \arg target задает нужный сенсор
   \arg dr возвращает сырое отношение сопротивлений сенсора
   \arg conc возвращает концентрацию __титульного__ газа сенсора */
static int _read_one(mics6814_sensor_t target, float * dr, float * conc)
{
	int error = 0;

	float r0;
	float rb;
	float a, exp_b;
	its_pld_analog_target_t analog_target;

	// Определяемся с параметрами сенсора
	switch (target)
	{
	case MICS6814_SENSOR_CO:
		r0 = MICS6814_CO_R0;
		rb = MICS6814_CO_RB;
		a  = MICS6814_CO_COEFFS_A;
		exp_b  = MICS6814_CO_COEFFS_EXP_B;
		analog_target = ITS_PLD_ANALOG_TARGET_MICS6814_CO;
		break;

	case MICS6814_SENSOR_NO2:
		r0 = MICS6814_NO2_R0;
		rb = MICS6814_NO2_RB;
		a  = MICS6814_NO2_COEFFS_A;
		exp_b  = MICS6814_NO2_COEFFS_EXP_B;
		analog_target = ITS_PLD_ANALOG_TARGET_MICS6814_NO2;
		break;

	case MICS6814_SENSOR_NH3:
		r0 = MICS6814_NH3_R0;
		rb = MICS6814_NH3_RB;
		a  = MICS6814_NH3_COEFFS_A;
		exp_b  = MICS6814_NH3_COEFFS_EXP_B;
		analog_target = ITS_PLD_ANALOG_TARGET_MICS6814_NH3;
		break;

	default:
		error = -EINVAL;
		break;
	}

	if (0 != error)
		return error;


	// делаем замер через АЦП
	// Несколько замеров, чтобы фильтрануть шум
	uint16_t raw;
	uint32_t raw_sum = 0;
	const int oversampling = 10;
	for (int i = 0; i < oversampling; i++)
	{
		error = its_pld_analog_get_raw(analog_target, &raw);
		if (0 != error)
			return error;

		raw_sum += raw;
	}

	raw = raw_sum / oversampling;
	// Считаем сопротивление сенсора
	float rx = rb * (float)raw/(0x0FFF - raw); // 0x0FFF - потолок нашего АЦП

	// Пересчитываем в концентрацию
	*dr = rx/r0;
	*conc = pow(*dr, a) * exp_b;

	return 0;
}



int mics6814_read(mavlink_pld_mics_6814_data_t * msg)
{
	// Берем время
	its_time_t time;
	its_gettimeofday(&time);

	msg->time_s = time.sec;
	msg->time_us = time.usec;

	// Теперь опрашиваем все сенсоры
	int error;
	error = _read_one(MICS6814_SENSOR_CO, &msg->red_sensor_raw, &msg->co_conc);
	if (0 != error)
		return error;

	error = _read_one(MICS6814_SENSOR_NO2, &msg->ox_sensor_raw, &msg->no2_conc);
	if (0 != error)
		return error;

	error = _read_one(MICS6814_SENSOR_NH3, &msg->nh3_sensor_raw, &msg->nh3_conc);
	if (0 != error)
		return error;

	return 0;
}
