#include "mics6814.h"

#include <math.h>
#include <errno.h>

#include <stm32f1xx_hal.h>

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
#define MICS6814_CO_COEFFS_B	(1.1586944311785252)

// Значение R0 (сопротивление сенсора в калибровочной атмосфере)
#define MICS6814_CO_R0	(100)

// Номинал Балансирующего резистора R1 из резисторного делителя
#define MICS6814_CO_R1_VALUE	(10*1000)

// Второй порт/пин для управления балансирующими резисторами
#define MICS6814_CO_CTRL_2_PORT		GPIOB
#define MICS6814_CO_CTRL_2_PIN		GPIO_PIN_2

// Номинал Балансирующего резистора R2 из резисторного делителя
#define MICS6814_CO_R2_VALUE	(10*1000)

// Первый порт/пин для управления балансирующими резисторами
#define MICS6814_CO_CTRL_1_PORT		GPIOB
#define MICS6814_CO_CTRL_1_PIN		GPIO_PIN_1

// Номинал Балансирующего резистора R3 из резисторного делителя
#define MICS6814_CO_R3_VALUE	(10*1000)

// Конценрация CO из сопротивления RED сенсора MICS-6814
static float _rescale_co(float red_r)
{
	float dr = red_r / MICS6814_CO_R0;
	return pow(dr, MICS6814_CO_COEFFS_A) * exp(MICS6814_CO_COEFFS_B);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Сенсор NO2 (OX)
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Коэфициенты пересчёта в ppm
/* Посчитаны по графику в даташите из двух точек:
 * x1 = 20, y1 = 3
 * x2 = 0.3, y2 = 0.05 */
#define MICS6814_NO2_COEFFS_A	(0.9749124012986611)
#define MICS6814_NO2_COEFFS_B	(-1.8219642557903095)

// Значение R0 (сопротивление сенсора в калибровочной атмосфере)
#define MICS6814_NO2_R0	(100)

// Номинал Балансирующего резистора R1 из резисторного делителя
#define MICS6814_NO2_R1_VALUE	(10*1000)

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

// Концентрация NO2 из сопротивления OX сенсора MICS-6814
static float _rescale_no2(float ox_r)
{
	float dr = ox_r / MICS6814_NO2_R0;
	return pow(dr, MICS6814_NO2_COEFFS_A) * exp(MICS6814_NO2_COEFFS_B);
}



// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Сенсор NH3
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Коэфициенты пересчёта в ppm
/* Посчитаны по графику в даташите из двух точек:
 * x1 = 0.8, y1 = 1
 * x2 = 0.08, y2 = 70*/
#define MICS6814_NH3_COEFFS_A	(-1.845098040014257)
#define MICS6814_NH3_COEFFS_B	(-0.4117217291716697)

// Значение R0 (сопротивление сенсора в калибровочной атмосфере)
#define MICS6814_NH3_R0	(100)

// Номинал Балансирующего резистора R1 из резисторного делителя
#define MICS6814_NH3_R1_VALUE	(10*1000)

// Второй порт/пин для управления балансирующими резисторами
#define MICS6814_NH3_CTRL_2_PORT	GPIOA
#define MICS6814_NH3_CTRL_2_PIN		GPIO_PIN_6

// Номинал Балансирующего резистора R2 из резисторного делителя
#define MICS6814_NH3_R2_VALUE	(10*1000)

// Первый порт/пин для управления балансирующими резисторами
#define MICS6814_NH3_CTRL_1_PORT	GPIOA
#define MICS6814_NH3_CTRL_1_PIN		GPIO_PIN_7

// Номинал Балансирующего резистора R3 из резисторного делителя
#define MICS6814_NH3_R3_VALUE	(10*1000)

// Общая функция пересчета значений
/*
 * r - текущее сопротивление сенсора
 * r0 - сопротивление сенсора в калибровочной атмосфере
 * a, b - коэффициенты пересчёта
 *
 * returns - значение ppm концентрации */
static float _rescale(float r, float r0, float a, float b)
{
	float dr = r / r0;
	return pow(dr, a) * exp(a);
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Управление верхним плечем делителя
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//! Количество резисторов в верхнем плече резисторного делителя сенсоров
typedef enum mics6814_upper_divider_t {
	//! Используются три резистора в плече
	MICS6814_DIVIER_UPPER_HALF_TRIPLE,
	//! Используются два резистора в плече
	MICS6814_DIVIER_UPPER_HALF_DOUBLE,
	//! Используется один резистор в плече
	MICS6814_DIVIER_UPPER_HALF_SINGLE,
} mics6814_divider_upper_half_t;


//! С каким именно сенсором мы работаем
typedef enum mics6814_sensor_t {
	MICS6814_SENSOR_CO,
	MICS6814_SENSOR_NO2,
	MICS6814_SENSOR_NH3,
} mics6814_sensor_t;


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


// Возвращает true, если пин в состоянии input floating
/* Поведение при pin == 0 не определено */
static bool _pin_is_input(GPIO_TypeDef * port, uint32_t pin)
{
	// Проходим по младшим битам
	uint8_t lower_pin_mask = (uint8_t)(pin & 0xFF);
	for (int i = 0; i < 8; i++)
	{
		if (0 == ((lower_pin_mask >> i) & 0x01))
			continue; // Этот бит нас не интересует

		// Режим порта задают два вот этих бита
		const uint8_t pin_mode_value = (port->CRL >> (4*i + 2)) & 0x03;
		// Если эти два бита имеют значение 0x01, значит пин в состоянии input floating
		if (0x01 != pin_mode_value)
			return false;
	}


	uint8_t upper_pin_mask = (uint8_t)((pin >> 8) & 0xFF);
	for (int i = 0; i < 8; i++)
	{
		if (0 == ((upper_pin_mask >> i) & 0x01))
			continue; // Этот бит нас не интересует

		// Режим порта задают два вот этих бита
		const uint8_t pin_mode_value = (port->CRH >> (4*i + 2)) & 0x03;
		// Если эти два бита имеют значение 0x01, значит пин в состоянии input floating
		if (0x01 != pin_mode_value)
			return false;
	}

	// Если все тесты прошли, то все ок
	return true;
}


// Возвращает _координаты_ управляющих пинов для управления плечами резисторного делителя
static int _divider_ctrl_gpio_for_target(mics6814_sensor_t target,
		GPIO_TypeDef ** port1, uint32_t * pin1,
		GPIO_TypeDef ** port2, uint32_t * pin2
)
{
	int error = 0;

	switch (target)
	{
	case MICS6814_SENSOR_CO:
		*port1 = MICS6814_CO_CTRL_1_PORT;
		*pin1 = MICS6814_CO_CTRL_1_PIN;
		*port2 = MICS6814_CO_CTRL_2_PORT;
		*pin2 = MICS6814_CO_CTRL_2_PIN;
		break;

	case MICS6814_SENSOR_NO2:
		*port1 = MICS6814_NO2_CTRL_1_PORT;
		*pin1 = MICS6814_NO2_CTRL_1_PIN;
		*port2 = MICS6814_NO2_CTRL_2_PORT;
		*pin2 = MICS6814_NO2_CTRL_2_PIN;
		break;

	case MICS6814_SENSOR_NH3:
		*port1 = MICS6814_NH3_CTRL_1_PORT;
		*pin1 = MICS6814_NH3_CTRL_1_PIN;
		*port2 = MICS6814_NH3_CTRL_2_PORT;
		*pin2 = MICS6814_NH3_CTRL_2_PIN;
		break;

	default:
		error = -ENOSYS;
		break;
	};

	return error;
}


// Текущие режимы верхнего плеча делителя для сенсоров
static mics6814_divider_upper_half_t _co2_mode;
static mics6814_divider_upper_half_t _no2_mode;
static mics6814_divider_upper_half_t _nh3_mode;


// Устанавливаем верхнее плечо делителя для указанного сенсора
int _divider_upper_half_set_mode(mics6814_sensor_t target, mics6814_divider_upper_half_t divider)
{
	int error;
	GPIO_TypeDef * port1, * port2;
	uint32_t pin1, pin2;

	error = _divider_ctrl_gpio_for_target(target, &port1, &pin1, &port2, &pin2);
	if (0 != error)
		return error;

	switch (divider)
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

	return error;
}


// Считаем величину верхнего плеча резисторного делителя для указанного сенсора
static int _divider_upper_half_get_mode(mics6814_sensor_t target, mics6814_divider_upper_half_t * mode)
{
	GPIO_TypeDef * port1, * port2;
	uint32_t pin1, pin2;

	int error = _divider_ctrl_gpio_for_target(target, &port1, &pin1, &port2, &pin2);
	if (0 != error)
		return error;


	if (_pin_is_input(port1, pin1))
	{
		if (_pin_is_input(port2, pin2))
		{
			// Оба управляющих пина отключены
			*mode = MICS6814_DIVIER_UPPER_HALF_TRIPLE;
		}
		else
		{
			// Первый пин отключен, второй пин в питании. Работают два резистора
			*mode = MICS6814_DIVIER_UPPER_HALF_DOUBLE;
		}
	}
	else
	{
		// Первый пин в питании. Полюбому работает один резистор
		*mode = MICS6814_DIVIER_UPPER_HALF_SINGLE;
	}

	return 0;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Работа с сенсором по-крупному
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


int mics6814_init(void)
{
	// Нужно настроить GPIO для упрваления плечами
	// но они правильно настроены с резета (в in)
	// поэтому мы их не трогаем тоже
	// Только включим клоки (хотя они и так должны быть включены)
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	// Все резисторы сверху
	_co2_mode = MICS6814_DIVIER_UPPER_HALF_TRIPLE;
	_no2_mode = MICS6814_DIVIER_UPPER_HALF_TRIPLE;
	_nh3_mode = MICS6814_DIVIER_UPPER_HALF_TRIPLE;
}



int mics6814_read(mavlink_pld_mics_6814_data_t * msg)
{
	// Читаем значения АЦП
	float raw_co = its_pld_analog_get_mv(target, value);
}
