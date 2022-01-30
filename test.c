#include <stdio.h>
#include <string.h>
#include "SharedMemory.h"

int main(void){
    char memory[1024];
    memset(&memory[0], 0, sizeof(memory));

    // инициализирую
    sm_init(&memory);

    printf("init cnt = %d\n\n", sm_get_number_rec());

    char out_data[1024];

    ///////////////////////////////////////////////////////
    // пишу
    for(uint32_t i = 1; i <= 10; i++){
        memset(&out_data[0], 0, sizeof(out_data));
        sprintf(&out_data[0], "%s %d", "test", i);
        sm_add_data(strlen(&out_data[0]), &out_data[0]);
    }

    // еще пишу
    sm_add_data(strlen("hello world!"), "hello world!");

    // пишу цифру
    int i = 1234567890;
    sm_add_data(sizeof(int), &i);

    ///////////////////////////////////////////////////////
    // читаю 
    uint32_t len;
    uint32_t cnt = sm_get_number_rec();
    
    for(uint32_t i = 1; i <= (cnt - 1); i++){
        memset(&out_data[0], 0, sizeof(out_data));
        len = sm_read_data(i, &out_data[0]);
        printf("data len = %d, %s\n", len, &out_data[0]);
    }

    // читаю цифру
    int out_i;
    len = sm_read_data(sm_get_number_rec(), &out_i);
    printf("data len = %d, %d\n", len, out_i);

    printf("cnt = %d\n\n", sm_get_number_rec());

    ///////////////////////////////////////////////////////
    // удаляю 
    sm_delete_data(sm_get_number_rec()-1);
    //sm_delete_data(sm_get_number_rec());
    sm_delete_data(1);
    sm_delete_data(3);

    //////////////////////////////////////////////////////
    // читаю 
    
    cnt = sm_get_number_rec();

    for(uint32_t i = 1; i <= (cnt - 1); i++){
        memset(&out_data[0], 0, sizeof(out_data));
        len = sm_read_data(i, &out_data[0]);
        printf("data len = %d, %s\n", len, &out_data[0]);
    }

    // читаю цифру
    len = sm_read_data(sm_get_number_rec(), &out_i);
    printf("data len = %d, %d\n", len, out_i);

    printf("cnt = %d\n", cnt);

    char symbol;
    scanf("%c", &symbol);

    return 0;
}