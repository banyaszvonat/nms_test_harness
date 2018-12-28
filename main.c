#include <windows.h>
#include <stdio.h>
//#include "Filter.h"

typedef int (*PFILTERSETALLOC)(void *, void *, void *, void *);
typedef int (*PFILTERCREATE)(void *, long int, void *, long int);
typedef size_t (*PFILTERGETBUFFERSIZE)(void*, char *);
typedef int (*PFILTERFILTER)(void *, char*, char *, size_t);

PFILTERSETALLOC ProfanityFilter_SetAllocator;
PFILTERCREATE ProfanityFilter_Create;
PFILTERGETBUFFERSIZE ProfanityFilter_GetRequiredBufferSize;
PFILTERFILTER ProfanityFilter_Filter;

HANDLE dictionary_source;
HANDLE dictionary_src_file;

int harness_load_symbols()
{
	LPVOID errorMsg;
	HMODULE profanity_filter = LoadLibraryA("ProfanityFilter_x64_v120");
	DWORD lastError;


	if(!profanity_filter)
	{
		lastError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorMsg, 0, NULL);
		fprintf(stderr, "Error loading library: %s\n", (char *) errorMsg);
		return 1;
	}


	ProfanityFilter_SetAllocator = (PFILTERSETALLOC) GetProcAddress(profanity_filter, "ProfanityFilter_SetAllocator");
	
	if(!ProfanityFilter_SetAllocator)
	{
		lastError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorMsg, 0, NULL);
		fprintf(stderr, "Error loading function SetAllocator: %s\n", (char *) errorMsg);
		return 2;
	}

	ProfanityFilter_Create = (PFILTERCREATE) GetProcAddress(profanity_filter, "ProfanityFilter_Create");

	if(!ProfanityFilter_Create)
	{
		lastError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorMsg, 0, NULL);
		fprintf(stderr, "Error loading function Create: %s\n", (char *) errorMsg);
		return 2;
	}

	ProfanityFilter_GetRequiredBufferSize = (PFILTERGETBUFFERSIZE) GetProcAddress(profanity_filter, "ProfanityFilter_GetRequiredBufferSize");

	if(!ProfanityFilter_GetRequiredBufferSize)
	{
		lastError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorMsg, 0, NULL);
		fprintf(stderr, "Error loading function GetRequiredBufferSize: %s\n", (char *) errorMsg);
		return 2;
	}

	ProfanityFilter_Filter = (PFILTERFILTER) GetProcAddress(profanity_filter, "ProfanityFilter_Filter");

	if(!ProfanityFilter_Filter)
	{
		lastError = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &errorMsg, 0, NULL);
		fprintf(stderr, "Error loading function Filter: %s\n", (char *) errorMsg);
		return 2;
	}



	if(!ProfanityFilter_GetRequiredBufferSize || !ProfanityFilter_Create || !ProfanityFilter_Filter)
	{
		fprintf(stderr, "Error getting symbol address: %x\n", GetLastError());
		return 3;
	}
	else
	{
		return 0;
	}
}

void * harness_load_dictionary()
{
	//dictionary_source = OpenFileMappingA(FILE_MAP_READ, true, "dictionary_source.bin");
		
	if((dictionary_src_file = CreateFileA("dictionary_source.bin", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != (HANDLE) INVALID_HANDLE_VALUE)
	{
		dictionary_source = CreateFileMapping(dictionary_src_file, NULL, PAGE_READONLY, 0, 0, NULL);
		return MapViewOfFile(dictionary_source, FILE_MAP_READ, 0, 0, 0);
	}
	else
	{
		return NULL;
	}
}

int harness_initialize_filter(void *dictionary_src_ptr, void **dictionary_dst_ptr)
{
	int retVal;
	//fprintf(stderr, "SetAllocator executing...\n");
	//retVal = ProfanityFilter_SetAllocator(NULL, NULL, NULL, NULL);
	//fprintf(stderr, "SetAllocator returned: 0x%x\n", retVal);
	fprintf(stderr, "Create executing...\n");
	retVal = ProfanityFilter_Create(dictionary_src_ptr, 70972, dictionary_dst_ptr, 384);
	fprintf(stderr, "Create returned: 0x%x\n", retVal);
	return retVal;
	//ProfanityFilter_SetAllocator(NULL, NULL, NULL, NULL);
	//save alloc pointers
	//ProfanityFilter_CreateFilter()
}

size_t harness_buffersize(void * dictionary, char * test_input)
{
	fprintf(stderr, "GetRequiredBufferSize executing...\n");
	size_t bufsize = ProfanityFilter_GetRequiredBufferSize(dictionary, test_input);
	fprintf(stderr, "GetRequiredBufferSize returned: %zd\n", bufsize);
	return bufsize;
}

int harness_filter(void * dictionary, char * test_input, char * output, size_t buffersize)
{
	//fprintf(stderr, "GetRequiredBufferSize executing...\n");
	//int bufsize = ProfanityFilter_GetRequiredBufferSize(dictionary, test_input);
	//fprintf(stderr, "GetRequiredBufferSize returned: %d\n", bufsize);
	//TODO: do this without casting to size_t later
	//output = malloc((size_t) bufsize);
	//return 0;
	//TODO: figure out arguments here
	int retVal;

	fprintf(stderr, "Filter executing...\n");
	retVal = ProfanityFilter_Filter(dictionary, test_input, output, buffersize);
	return retVal;
}


int main(int argc, char **argv)
{
	void *dictionary_src_ptr;
	void *dictionary; // dictionary is allocated by the library, so we don't malloc() here. we just need a pointer to hold the address ProfanityFilter_Create() gives
	char *output;
	
	FILE *inputfile;
	char * filecontents;
	long length;
	size_t buffersize = 0;

	if(argc < 2)
	{
		printf("Usage: %s inputfile\n", argv[0]);
		return 1;
	}


	fprintf(stderr, "loading symbols...\n");
	if(harness_load_symbols() > 0)
	{
		printf("symbol load fail\n");
		return 2;
	}

	fprintf(stderr, "loading dictionary...\n");
	if((dictionary_src_ptr = harness_load_dictionary()) == NULL)
	{
		fprintf(stderr, "load dict fail\n");
		CloseHandle(dictionary_source);
		CloseHandle(dictionary_src_file);
		return 3;
	}

	fprintf(stderr, "initializing filter...\n");
	if(harness_initialize_filter(dictionary_src_ptr, &dictionary) > 0)
	{
		fprintf(stderr, "filter init fail\n");
		CloseHandle(dictionary_source);
		CloseHandle(dictionary_src_file);
		return 4;
	}

	// open test input file and read contents
	inputfile = fopen(argv[1], "rb");

	if(inputfile)
	{
		fseek(inputfile, 0, SEEK_END);
		length = ftell(inputfile);
		fseek(inputfile, 0, SEEK_SET);
		filecontents = malloc(length+1);

		fread(filecontents, length, 1, inputfile);
		fclose(inputfile);

		filecontents[length] = 0;
	}

	//we already know the buffer size, but let's consult the library too
	buffersize = harness_buffersize(dictionary, filecontents);
	output = malloc(buffersize);

	if(harness_filter(dictionary, filecontents, output, buffersize) > 0)
	{
		printf("not filtered: %s\n", output);
	}
	else
	{
		printf("Filtered or something: %s\n", output);
	}

	free(output);
	CloseHandle(dictionary_source);
	CloseHandle(dictionary_src_file);
	return 0;
}