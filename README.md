Texture - преобразование форматов изображений
=============================================

Программа поддерживает чтение изображений и текстур следующих форматов:
* TARGA,
* BMP,
* LMP Quake,
* LMP Half-Life,
* MIP Quake,
* MIP Half-Life,
* M8 Heretic 2,
* M32 Heretic 2,
* WAD Quake,
* WAD Half-Life,
* WAL Quake 2.

И поддерживает вывод изображений в следующие форматы:
* TARGA,
* BMP,
* LMP Quake,
* LMP Half-Life,
* MIP Quake,
* MIP Half-Life,
* M8 Heretic 2,
* M32 Heretic 2,
* WAL Quake 2.

Программа написана в 2002 году и теперь мне кажется не такой большой, какой казалась в момент написания. Сейчас она кажется мне написанной довольно не аккуратно: не у всех фукнций контролируется код возврата, не выводятся отладочные сообщения, не всегда корректно проверяется соответствие файла формату, строковые функции не проверяют размеров буферов.

Ограничился правками, позволяющими собрать программу в Linux, стилистическими правками, внёс несколько дополнений в обработку ошибок, в разбор параметров и в обработку строк.

Для сборки программы в Windows нужно определить в опциях компилятора макрос WINDOWS, который заменит в программе разделитель каталогов на обратную косую черту - \\.

(C) 2002-2018 Владимир Ступин

Программа распространяется под лицензией GPL 3.
