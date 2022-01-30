#ifndef __SHARED_MEMORY_H
#define __SHARED_MEMORY_H

#include <stdint.h>

/**
 * @brief инициализирует расположение данных в памяти
 * 
 * @param addr - начальный адрес расположения данных 
 */
void sm_init(void *addr);

/**
 * @brief возращает общее количество элементов данных
 * 
 * @return int 
 */
int sm_get_number_rec(void);

/**
 * @brief добавляет данные в конец память
 * 
 * @param cnt - размер в байтах данных
 * @param data - указатель на данные
 */
void sm_add_data(uint32_t len, void *in_data);

/**
 * @brief читает данные из памяти по указанной позиции
 * 
 * @param poz - позиция данных в потоке начинается с 1
 * @param out_data - указатель на выходные данные
 * @return uint32_t - возращает длинну данных в байтах
 */
uint32_t sm_read_data(uint32_t poz, void *out_data);

/**
 * @brief  удаляет данные по указанной позиции
 * 
 * @param poz позиция записи данных, начинается с 1
 */
void sm_delete_data(uint32_t poz);

#endif