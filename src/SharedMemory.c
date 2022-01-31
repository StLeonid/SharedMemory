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
    bool check_crc;     // проверять или нет целостность данных
    uint32_t crc;       // сохранение CRC
    size_t size;        // размер пула данных
    sm_cnt_element cnt; // количество элементов данных
};

typedef struct _sms_t *_sms_p_t;
static _sms_p_t sm;

typedef uint32_t crc;

#define WIDTH (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0xD8 /* 11011 followed by 0's */

/**
 * @brief 
 * 
 * @param message - входные данные
 * @param nBytes - размер входных данных в байтах
 * @return crc - возвращает расчитаное число CRC
 */
__attribute__((weak)) crc _crcSlow(uint8_t const message[], int nBytes)
{
    crc remainder = 0;

    for (int byte = 0; byte < nBytes; ++byte)
    {
        remainder ^= (message[byte] << (WIDTH - 8));

        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (remainder & TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    return (remainder);
}

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
        size_t *ptr_size = (size_t *)ptr;
        ptr += (*ptr_size) + sizeof(size_t);
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
sm_status sm_init(void *addr, size_t size, bool crc)
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
        sm->check_crc = crc;
        if (sm->check_crc)
        {
            size_t size = sm_get_data_size();
            uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;
            sm->crc = _crcSlow(ptr, size);
        }
    }

    return SM_SUCCESS;
}

/**
 * @brief зануляет выделеную память
 * 
 * @return sm_status - возвращает статус выполнения
 */
sm_status sm_deinit(void)
{
    if (sm == NULL)
    {
        return ADDR_ERROR;
    }

    if (sm->magic != MAGIC_NUMBER)
    {
        size_t size = sm_get_data_size();
        uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;
        memset(ptr, 0, size);
        memset(sm, 0, sizeof(struct _sms_t));
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

    if (sm->magic != MAGIC_NUMBER)
    {
        return len_all;
    }

    sm_cnt_element cnt = sm_get_number_rec();

    for (sm_cnt_element i = 0; i < cnt; i++)
    {
        size_t *ptr_n = (size_t*)_seek_data(i);

        if (ptr_n != NULL)
        {
            len_all += *ptr_n + sizeof(size_t);
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
sm_status sm_add_data(size_t len, void *in_data)
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

    // проверяем целостность данных перед записью
    crc CRC = 0;
    if (sm->check_crc)
    {
        uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;
        CRC = _crcSlow(ptr, size);
        if (sm->crc != CRC)
        {
            return CRC_ERROR;
        }
    }

    // смещаемся на конец данных
    size_t *ptr = (size_t*)_seek_data(sm->cnt);

    if (ptr != NULL)
    {
        *ptr = len;
        ptr++;
        memcpy(ptr, (uint8_t *)in_data, len);
        sm->cnt++;

        // если требуется обновляем CRC
        if (sm->check_crc)
        {
            size = sm_get_data_size();
            uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;
            sm->crc = _crcSlow(ptr, size);
        }
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
 * @return size_t - возращает длинну данных в байтах
 */
size_t sm_read_data(sm_cnt_element poz, void *out_data)
{
    size_t len = 0;

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

    // проверяем целостность данных перед записью
    crc CRC = 0;
    if (sm->check_crc)
    {
        size_t size = sm_get_data_size();
        uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;
        CRC = _crcSlow(ptr, size);
        if (sm->crc != CRC)
        {
            return len;
        }
    }

    size_t *ptr = (size_t*)_seek_data(poz);

    if (ptr != NULL)
    {
        len = *ptr;
        ptr++;
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

    size_t len_all = 0;
    size_t *ptr_n;

    for (sm_cnt_element i = poz; i < sm->cnt; i++)
    {
        ptr_n = (size_t*)_seek_data(i);

        if (ptr_n != NULL)
        {
            len_all += *ptr_n + sizeof(size_t);
        }
    }

    // указатель на следущие данные после удаляемых
    ptr_n = (size_t*)_seek_data((poz + 1));

    if (ptr_n != NULL)
    {
        // копируем хвост на место удаленных
        memcpy(ptr_del, ptr_n, len_all);
        sm->cnt--;

        // если требуется обновляем CRC
        if (sm->check_crc)
        {
            size_t size = sm_get_data_size();
            uint8_t *ptr = (uint8_t *)sm + sizeof(struct _sms_t) + 1;
            sm->crc = _crcSlow(ptr, size);
        }
    }
    else
    {
        return DATA_ERROR;
    }

    return SM_SUCCESS;
}
