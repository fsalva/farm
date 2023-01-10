
typedef struct file
{
    char * filename;
    int result;

} file;

int compare_elements(file * a, file * b);


file * createFile(char * filename, int res);

