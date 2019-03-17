#include "file_io.h"

#include "logger.h"

File_Result load_file(const char* file_name, Memory_Arena* arena)
{
    File_Result result = {};

    FILE* file;
	
    file = fopen(file_name, "rb");
    if(file == NULL) 
    {
        log_warning("Unable to open file: %s", file_name);
        return result;
    }

    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    fseek(file, 0, SEEK_SET);

	if(arena != NULL)
    	result.contents = (char*)push_memory(arena, sizeof(char) * result.size + 1);
	else 
		result.contents = (char*)malloc(sizeof(char) * result.size + 1);

    memset(result.contents, 0, result.size + 1);
    if(fread(result.contents, 1, result.size, file) != result.size)
        log_warning("Unable to read file: %s", file_name);
    
    fclose(file);

    log_info("Loaded file: %s", file_name);
    return result;
}

File_Result load_temp_file(const char* file_name)
{
	return load_file(file_name, NULL);
}

Text_File_Handler get_text_file_handler(File_Result* file_result)
{
    Text_File_Handler text_handler = {};
    text_handler.at = file_result->contents;
    text_handler.length = file_result->size;
	
	// always consume the first word
	consume_word(&text_handler);

	return text_handler;
}

bool is_end_of_line(char c)
{
	bool result = 
		((c == '\n') ||
		(c == '\r'));
					
	return result;
}

bool is_white_space(char c)
{
	bool result = 
		((c == ' ') ||
		(c == '\t') ||
		(c == '\v') ||
		(c == '\f') ||
		is_end_of_line(c));
					
	return result;
}

bool is_alpha(char c)
{
	bool result = (((c >= 'a') && (c <= 'z')) ||
					
		((c >= 'A') && (c <= 'Z')));

	return result;
}

bool is_number(char c)
{
	bool result = ((c >= '0') && (c <= '9'));

	return result;
}

bool is_ascii(char c)
{
	bool result = c < 127;

	return result;
}

void eat_all_white_space(Text_File_Handler* text_file_handler)
{
	for (;;)
	{
		if (is_white_space(text_file_handler->at[0]))
		{
			++text_file_handler->at;
		}
		else if ((text_file_handler->at[0] == '/') && (text_file_handler->at[1] == '/'))
		{
			text_file_handler->at += 2;
			while (text_file_handler->at[0] && !is_end_of_line(text_file_handler->at[0]))
			{
				++text_file_handler->at;
			}
		}
		else if ((text_file_handler->at[0] == '/') && (text_file_handler->at[1] == '*'))
		{
			text_file_handler->at += 2;
			while (text_file_handler->at[0] && 
				!((text_file_handler->at[0] == '*') && 
				(text_file_handler->at[1] == '/')))
			{
				++text_file_handler->at;
			}		

			if (text_file_handler->at[0] == '*')
			{
				text_file_handler->at += 2;
			}
		}
		else
		{
			break;
		}
	}
}

b32 can_consume_lines(Text_File_Handler* text_file_handler)
{
    return (*text_file_handler->at != NULL &&
		 (is_white_space(text_file_handler->at[0]) || is_alpha(text_file_handler->at[0]) || is_number(text_file_handler->at[0]))
		 	);
}

void consume_line(Text_File_Handler* text_file_handler)
{
    eat_all_white_space(text_file_handler);

    if(*text_file_handler->at == NULL)
    {
        text_file_handler->line_length = 0;
        return;
    }
    
    char* text = text_file_handler->at;

    while(!is_end_of_line(text_file_handler->at[0]))
        ++text_file_handler->at;       

    text_file_handler->line_length = text_file_handler->at - text;
    text_file_handler->line = text;
}

void consume_word(Text_File_Handler* text_file_handler)
{
    eat_all_white_space(text_file_handler);

    if(*text_file_handler->at == NULL)
    {
        text_file_handler->word_length = 0;
        return;
    }
    
    char* text = text_file_handler->at;

    while(!is_white_space(text_file_handler->at[0]))
        ++text_file_handler->at;       

    text_file_handler->word_length = text_file_handler->at - text;
    text_file_handler->word = text;
}

bool word_is(Text_File_Handler* text_file_handler, const char* match)
{
	char *at = (char*)match;
	for (int i = 0; i < text_file_handler->word_length; ++i, ++at)
	{
		if ((*at == 0) || (text_file_handler->word[i] != *at))
		{
			return false;
		}
	}

	bool result = (*at == 0);
	return result;
}
