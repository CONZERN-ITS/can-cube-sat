#!/usr/bin/env bash

# Определяем путь до родительского каталога этого скрипта
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Переходим в каталог к мавлинку
cd $DIR/pymavlink

# Очищаем результаты прошлой сборки
/usr/bin/env python3 setup.py clean --all

# Собираем whl
/usr/bin/env python3 setup.py bdist_wheel

# находим собранный whl в папке dist
wheel_path=$(find dist -name pymavlink*.whl)
if [ -z "$wheel_path" ]
then
	echo "Не могу найти собранный wheel для установки. Он не собрался?"
	exit 1
fi

# Устанавливаем собранный whl в текущий активный питон
/usr/bin/env pip3 install -U $wheel_path
