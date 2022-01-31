#include <stdio.h>
#include <string.h>
#include "SharedMemory.h"

int main(void)
{
    char memory[1024];
    memset(&memory[0], 0, sizeof(memory));

    // инициализирую
    sm_init(&memory, sizeof(memory));

    printf("init cnt = %d\n\n", sm_get_number_rec());

    char out_data[1024];

    ///////////////////////////////////////////////////////
    // пишу
    sm_status stat;
    for (uint32_t i = 0; i <= 10; i++)
    {
        memset(&out_data[0], 0, sizeof(out_data));
        sprintf(&out_data[0], "%s %d", "test", i);
        stat = sm_add_data(strlen(&out_data[0]), &out_data[0]);
        if (stat != SM_SUCCESS)
        {
            printf("error = %d", (int)stat);
        }
    }

    // еще пишу
    stat = sm_add_data(strlen("hello world!"), "hello world!");
    if (stat != SM_SUCCESS)
    {
        printf("error = %d", (int)stat);
    }

    // пишу цифру
    int i = 1234567890;
    stat = sm_add_data(sizeof(int), &i);
    if (stat != SM_SUCCESS)
    {
        printf("error = %d", (int)stat);
    }

    ///////////////////////////////////////////////////////
    // читаю
    uint32_t len;
    uint32_t cnt = sm_get_number_rec();
    size_t dataSize = sm_get_data_size();

    for (uint32_t i = 0; i < (cnt - 1); i++)
    {
        memset(&out_data[0], 0, sizeof(out_data));
        len = sm_read_data(i, &out_data[0]);
        printf("data len = %d, %s\n", len, &out_data[0]);
    }

    // читаю цифру
    int out_i;
    len = sm_read_data((sm_get_number_rec() - 1), &out_i);
    printf("data len = %d, %d\n", len, out_i);

    printf("cnt = %d, size = %d byte\n\n", cnt, (int)dataSize);

    ///////////////////////////////////////////////////////
    // удаляю
    stat = sm_delete_data(sm_get_number_rec() - 2);
    if (stat != SM_SUCCESS)
    {
        printf("error = %d", (int)stat);
    }

    stat = sm_delete_data(sm_get_number_rec() - 1);
    if (stat != SM_SUCCESS)
    {
        printf("error = %d", (int)stat);
    }

    stat = sm_delete_data(1);
    if (stat != SM_SUCCESS)
    {
        printf("error = %d", (int)stat);
    }

    stat = sm_delete_data(3);
    if (stat != SM_SUCCESS)
    {
        printf("error = %d", (int)stat);
    }

    //////////////////////////////////////////////////////
    // читаю

    cnt = sm_get_number_rec();
    dataSize = sm_get_data_size();

    for (uint32_t i = 0; i < (cnt - 1); i++)
    {
        memset(&out_data[0], 0, sizeof(out_data));
        len = sm_read_data(i, &out_data[0]);
        printf("data len = %d, %s\n", len, &out_data[0]);
    }

    // читаю цифру
    len = sm_read_data((sm_get_number_rec() - 1), &out_i);
    printf("data len = %d, %d\n", len, out_i);

    printf("cnt = %d, size = %d byte\n\n", cnt, (int)dataSize);

    char symbol;
    scanf("%c", &symbol);

    return 0;
}