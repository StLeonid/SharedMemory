#include "SharedMemory.h"
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#define MAGIC_NUMBER    (0xAA557777)
#define VERSION         (1)


struct _sms_t{
    uint32_t magic;
    uint32_t version;
    uint32_t crc;
    uint32_t cnt;
}sms_t;

typedef struct _sms_t *_sms_p_t;
static _sms_p_t sm;

/**
 * @brief инициализирует расположение данных в памяти
 * 
 * @param addr - начальный адрес расположения данных 
 */
void sm_init(void *addr){
    if(addr == NULL){
        return;
    }

    sm = addr;

    if(sm->magic != MAGIC_NUMBER){
        memset(sm, 0, sizeof(sms_t));
        sm->magic = MAGIC_NUMBER;
        sm->version = VERSION;
    }
}

/**
 * @brief возращает общее количество элементов данных
 * 
 * @return int - количество элементов данных
 */
int sm_get_number_rec(void){
    return sm->cnt;
}

/**
 * @brief смещается на указанную позицию в потоке файла
 * 
 * @param poz - номер позиции данных начинается с 0
 * @return uint8_t - указатель на данные
 */
static uint8_t* _seek_data(uint32_t poz){
    uint8_t *ptr = (uint8_t*)sm + sizeof(sms_t) + 1;

    for(uint32_t i = 0; i < poz; i++){
        ptr += (*ptr) + sizeof(uint32_t);
    }

    return ptr;
}

/**
 * @brief добавляет данные в конец память
 * 
 * @param cnt - размер данных в байтах
 * @param data - указатель на данные
 */
void sm_add_data(uint32_t len, void *in_data){
    if(len <= 0){
        return;
    }

    if(in_data == NULL){
        return;
    }

    if(sm->magic == MAGIC_NUMBER){
        uint8_t *ptr = _seek_data(sm->cnt);

        if(ptr != NULL){
            *(uint32_t*)ptr = len;
            ptr += sizeof(uint32_t);
            memcpy(ptr, (uint8_t*)in_data, len);
            sm->cnt++;
        }
    }
}

/**
 * @brief читает данные из памяти по указанной позиции
 * 
 * @param poz - позиция данных в потоке начинается с 1
 * @param out_data - указатель на выходные данные
 * @return uint32_t - возращает длинну данных в байтах
 */
uint32_t sm_read_data(uint32_t poz, void *out_data){
    uint32_t len = 0;

    if(out_data == NULL){
        return len;
    }

    if(poz == 0){
        return len;
    }

    if(sm->magic == MAGIC_NUMBER){
        if(sm->cnt < poz){
            return len;
        }

        uint8_t *ptr = _seek_data((poz - 1));

        if(ptr != NULL){
            len = *ptr;
            ptr += sizeof(uint32_t);
            memcpy(out_data, ptr, len);
        }
    }

    return len;
}

/**
 * @brief  удаляет данные по указанной позиции
 * 
 * @param poz позиция записи данных, начинается с 1
 */
void sm_delete_data(uint32_t poz){
    if(poz == 0){
        return;
    }

    if(sm->magic == MAGIC_NUMBER){
        uint8_t *ptr_del = _seek_data((poz - 1));

        if(ptr_del != NULL){
            // последняя позиция?
            if(sm->cnt == poz){       
                memset(ptr_del, 0, sizeof(uint32_t));
                sm->cnt--;
                return;
            }

            uint32_t len_all = 0;
            uint8_t *ptr_n;

            for(uint32_t i = poz; i < sm->cnt; i++){
                ptr_n = _seek_data(i);

                if(ptr_n != NULL){
                    len_all += *ptr_n + sizeof(uint32_t);
                }
            }

            // указатель на следущие данные после удаляемых
            ptr_n = _seek_data(poz);

            // копируем хвост нв место удаленных
            memcpy(ptr_del, ptr_n, len_all);
            sm->cnt--;
        }
    }
}
