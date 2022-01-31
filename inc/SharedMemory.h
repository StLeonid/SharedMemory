#ifndef __SHARED_MEMORY_H
#define __SHARED_MEMORY_H

#include <stdint.h>
#include <stdbool.h>

typedef enum{
    SM_SUCCESS,
    SM_MAGIC_ERROR,// не отформатирована память
    SM_CRC_ERROR,  // ошибка расчета CRC
    SM_ADDR_ERROR, // не корректно задан адрес пула памяти
    SM_DATA_ERROR, // не корректные данные для записи
    SM_SIZE_ERROR  // не хватка памяти для записи в пул памяти
}sm_status;

typedef uint32_t sm_cnt_element;

/**
 * @brief инициализирует расположение данных в памяти
 * 
 * @param addr - начальный адрес расположения данных 
 * @param size - размер пула данных
 */
sm_status sm_init(void *addr, size_t size, bool crc);

/**
 * @brief зануляет выделеную память
 * 
 * @return sm_status - возвращает статус выполнения
 */
sm_status sm_deinit(void);

/**
 * @brief возращает общее количество элементов данных
 * 
 * @return sm_cnt_element - количество элементов данных
 */
sm_cnt_element sm_get_number_rec(void);

/**
 * @brief возвращает общий объем потребеленной памяти в байтах
 * 
 * @return uint32_t - размер текущих данных в байтах
 */
size_t sm_get_data_size(void);

/**
 * @brief добавляет данные в конец память
 * 
 * @param len - размер данных в байтах
 * @param *in_data - указатель на данные
 * @return sm_status - статус выполнения
 */
sm_status sm_add_data(size_t len, void *in_data);

/**
 * @brief читает данные из памяти по указанной позиции
 * 
 * @param poz - позиция данных в потоке начинается с 0
 * @param out_data - указатель на выходные данные
 * @return size_t - возращает длинну данных в байтах
 */
size_t sm_read_data(sm_cnt_element poz, void *out_data);

/**
 * @brief  удаляет данные по указанной позиции
 * 
 * @param poz позиция записи данных, начинается с 0
 * @return sm_status - статус выполнения
 */
sm_status sm_delete_data(sm_cnt_element poz);

#endif