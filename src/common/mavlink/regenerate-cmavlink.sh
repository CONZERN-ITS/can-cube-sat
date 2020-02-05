#!/bin/bash

# Определяем путь до родительского каталога этого скрипта
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Работаем из родного каталога
cd "$DIR"

# Удаляем все что есть нагенеренного
rm -rf ./generated

# Вызываем питон в текущем каталоге и генерим C код из нашей XML-ки
/usr/bin/env python3 -m pymavlink.tools.mavgen \
	-o generated/mavlink \
	--lang=C \
	--wire-protocol=2.0 \
	./message_definitions/v1.0/its.xml