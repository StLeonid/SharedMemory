#include "SharedMemory.h"
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define MAGIC_NUMBER (0xAA557777)
#define VERSION (1)

struct _sms_t
{
    uint32_t magic;
    uint32_t version;
    uint32_t crc;
    size_t size;        // размер пула данных
    sm_cnt_element cnt; // количество элементов данных
};

typedef struct _sms_t *_sms_p_t;
static _sms_p_t sm;

/**
 * @brief смещается на указанную позицию в потоке файла
 * 
 * @param poz - номер позиции данных начинается с 0
 * @return uint8_t - указатель на данные
 */
static uint8_t *_seek_data(sm_cnt_element poz)
{
    uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;

    for (sm_cnt_element i = 0; i < poz; i++)
    {
        ptr += (*ptr) + sizeof(uint32_t);
    }

    return ptr;
}

/**
 * @brief инициализирует расположение данных в памяти
 * 
 * @param addr - начальный адрес расположения данных 
 * @param size - размер пула данных
 * @return sm_status - статус выполнения
 */
sm_status sm_init(void *addr, size_t size)
{
    if (addr == NULL)
    {
        return ADDR_ERROR;
    }

    sm = addr;

    if (sm->magic != MAGIC_NUMBER)
    {
        memset(sm, 0, sizeof(struct _sms_t));
        sm->magic = MAGIC_NUMBER;
        sm->version = VERSION;
        sm->size = size;
    }

    return SM_SUCCESS;
}

/**
 * @brief возращает общее количество элементов данных
 * 
 * @return sm_cnt_element - количество элементов данных
 */
sm_cnt_element sm_get_number_rec(void)
{
    return sm->cnt;
}

/**
 * @brief возвращает общий объем потребеленной памяти в байтах
 * 
 * @return size_t - размер текущих данных в байтах
 */
size_t sm_get_data_size(void)
{
    size_t len_all = 0;

    sm_cnt_element cnt = sm_get_number_rec();

    for (sm_cnt_element i = 0; i < cnt; i++)
    {
        uint8_t *ptr_n = _seek_data(i);

        if (ptr_n != NULL)
        {
            len_all += *ptr_n + sizeof(uint32_t);
        }
    }

    return len_all;
}

/**
 * @brief добавляет данные в конец память
 * 
 * @param len - размер данных в байтах
 * @param *in_data - указатель на данные
 * @return sm_status - статус выполнения
 */
sm_status sm_add_data(uint32_t len, void *in_data)
{
    if (sm->magic != MAGIC_NUMBER)
    {
        return MAGIC_ERROR;
    }

    if (len == 0)
    {
        return DATA_ERROR;
    }

    if (in_data == NULL)
    {
        return DATA_ERROR;
    }

    // расход памяти
    size_t size = sm_get_data_size();
    if ((size + len) > sm->size)
    {
        return SIZE_ERROR;
    }

    // смещаемся на конец данных
    uint8_t *ptr = _seek_data(sm->cnt);

    if (ptr != NULL)
    {
        *(uint32_t *)ptr = len;
        ptr += sizeof(uint32_t);
        memcpy(ptr, (uint8_t *)in_data, len);
        sm->cnt++;
    }
    else
    {
        return DATA_ERROR;
    }

    return SM_SUCCESS;
}

/**
 * @brief читает данные из памяти по указанной позиции
 * 
 * @param poz - позиция данных в потоке начинается с 0
 * @param out_data - указатель на выходные данные
 * @return uint32_t - возращает длинну данных в байтах
 */
uint32_t sm_read_data(sm_cnt_element poz, void *out_data)
{
    uint32_t len = 0;

    if (sm->magic != MAGIC_NUMBER)
    {
        return len;
    }

    if (out_data == NULL)
    {
        return len;
    }

    if (sm->cnt < (poz + 1))
    {
        return len;
    }

    uint8_t *ptr = _seek_data(poz);

    if (ptr != NULL)
    {
        len = *ptr;
        ptr += sizeof(uint32_t);
        memcpy(out_data, ptr, len);
    }

    return len;
}

/**
 * @brief  удаляет данные по указанной позиции
 * 
 * @param poz позиция записи данных, начинается с 0
 * @return sm_status - статус выполнения
 */
sm_status sm_delete_data(sm_cnt_element poz)
{
    if (sm->magic != MAGIC_NUMBER)
    {
        return MAGIC_ERROR;
    }

    if (sm->cnt < (poz + 1))
    {
        return DATA_ERROR;
    }

    uint8_t *ptr_del = _seek_data(poz);

    if (ptr_del == NULL)
    {
        return DATA_ERROR;
    }

    // последняя позиция?
    if (sm->cnt == (poz + 1))
    {
        memset(ptr_del, 0, sizeof(uint32_t));
        sm->cnt--;
        return SM_SUCCESS;
    }

    uint32_t len_all = 0;
    uint8_t *ptr_n;

    for (sm_cnt_element i = poz; i < sm->cnt; i++)
    {
        ptr_n = _seek_data(i);

        if (ptr_n != NULL)
        {
            len_all += *ptr_n + sizeof(uint32_t);
        }
    }

    // указатель на следущие данные после удаляемых
    ptr_n = _seek_data((poz + 1));

    if (ptr_n != NULL)
    {
        // копируем хвост на место удаленных
        memcpy(ptr_del, ptr_n, len_all);
        sm->cnt--;
    }
    else
    {
        return DATA_ERROR;
    }

    return SM_SUCCESS;
}
