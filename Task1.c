#include <stddef.h>


int32_t get_unique_num(int32_t *array, size_t len)
{
	int32_t res;

	// Для поиска не парного элемента массива за один проход используем XOR
	for(size_t i = 0; i < len; i++)
	{
		res ^= array[i];
	}

	return res;
}
