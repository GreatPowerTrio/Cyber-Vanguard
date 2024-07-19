#ifndef _COMM_H_
#define _COMM_H_

#include "main.h"


#define LENGTH 1000


extern char comm_string[LENGTH];
extern uint8_t comm_pointer;
extern bool comm_number_flag;

extern bool comm_load_flag;

typedef struct 
{
    char left_number;
    char right_number;
} number_message;

extern number_message comm_message;

void comm_get_date(uint8_t data);

bool comm_waiting_number(void);
bool comm_waiting_load(void);

#endif

