# Библиотека для хранения данных по указанному адресу и размеру памяти #

## Данные могут быть проверены на целостность при помощи CRC метода ##

## Встроеный слабый метод _crcSlow проверки CRC может быть заменен пользовательским ##

инициализирует расположение данных в памяти

__sm_status sm_init__

зануляет выделеную память

__sm_status sm_deinit__

возращает общее количество элементов данных

__sm_get_number_rec__

возвращает общий объем потребеленной памяти в байтах

__sm_get_data_size__

добавляет данные в конец памяти

__sm_add_data__

читает данные из памяти по указанной позиции

__sm_read_data__

удаляет данные по указанной позиции

__sm_delete_data__
