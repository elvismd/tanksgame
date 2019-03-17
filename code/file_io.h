#pragma once 

#include <stdio.h>
#include <stdlib.h>

#include "agnostic.h"
#include "memory_arena.h"

struct File_Result 
{
	char* contents;
	u32 size;
};

struct Text_File_Handler
{
	char* at;
	u32 length;

	char* word;
	u32 word_length;
	char* line;
	u32 line_length;
};

File_Result load_file(const char* file_name, Memory_Arena* arena);
File_Result load_temp_file(const char* file_name);

Text_File_Handler get_text_file_handler(File_Result* file_result);

bool is_end_of_line(char c);
bool is_white_space(char c);
bool is_alpha(char c);
bool is_number(char c);

void eat_all_white_space(Text_File_Handler* text_file_handler);
b32 can_consume_lines(Text_File_Handler* text_file_handler);
void consume_line(Text_File_Handler* text_file_handler);
void consume_word(Text_File_Handler* text_file_handler);
bool word_is(Text_File_Handler* text_file_handler, const char* match);