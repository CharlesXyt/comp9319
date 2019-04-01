#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

unsigned long file_length;
unsigned long** occ_table;
unsigned long* c_table;
unsigned char special_c_temp;
unsigned char special_c;
unsigned long* position;
FILE *fp_in;
unsigned long buck_interval = 1<<7;

typedef struct trace_t{
    unsigned long pos;
    unsigned char pos_c;
    int flag;
}trace_t;

typedef struct result_arr{
    unsigned long *array;
    unsigned long length;
}result_arr;

unsigned char read_file(unsigned long a){
    if(a >= file_length){
        a-= file_length;
    }
    fseek(fp_in, a, SEEK_SET);
    unsigned char c =fgetc(fp_in);
    if(c == special_c_temp)
        return special_c;
    return c;
}



int compare (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}

unsigned long get_length_file(FILE *fp){
    unsigned long length = 0;
    rewind(fp);
    while(1){
        if(fgetc(fp) == EOF){
            break;
        }
        length++;
    }
    return length;
}

unsigned long* make_ctable(FILE *fp){
    unsigned long *a = malloc(sizeof(unsigned long)*128);
    unsigned long *result = malloc(sizeof(unsigned long)*128);
    for(int i= 0;i<128;i++){
        a[i] = 0;
        result[i] =0;
    }
    unsigned char c;
    rewind(fp);
    unsigned long count = 0;
    while(count < file_length+1){
        c = fgetc(fp);
        if(c == special_c_temp)
            c = special_c;
        a[c]++;
	count++;
    }
    for(int i =0;i<128;i++){
        for(int j=0;j<i;j++){
            if(a[i]){
                result[i] += a[j];
            }else{
                break;
            }
        }
    }
    return result;
}

unsigned long* file_in_table(int i){
    unsigned long m = i*buck_interval;
    unsigned long* temp = malloc(sizeof(unsigned long)*128);
    if(i == 0){
        for(int j =0;j<128;j++){
            temp[j] =0;
        }
    }else{
        for(int j =0;j<128;j++){
            temp[j] = occ_table[i-1][j];
        }
        for(unsigned long begin = m - buck_interval;begin < m;begin++){
            if(begin >= file_length){
                break;
            }
            unsigned char c =fgetc(fp_in);
            if(c == special_c_temp){
                c = special_c;
            }
            temp[c]++;
        }
    }
    return temp;
}

void initial_occ_table(){
    rewind(fp_in);
    unsigned long bucket_length = file_length / buck_interval;
    occ_table = malloc(sizeof(unsigned long*)*bucket_length);
    for(int i =0;i<bucket_length+1;i++){
        occ_table[i] = file_in_table(i);
    }
}

unsigned long get_occ(unsigned char c,unsigned long pos){
    if(pos == ULONG_MAX){
        return 0;
    }
    unsigned long pos_buc = pos / buck_interval;
    unsigned long pos_occ = (pos % buck_interval) +1;
    unsigned long* temp = malloc(sizeof(unsigned long)*128);
    for(int j =0;j<128;j++){
        temp[j] = occ_table[pos_buc][j];
    }
    fseek(fp_in,pos_buc*buck_interval, SEEK_SET);
    for(unsigned long begin = pos_buc*buck_interval;begin < pos_buc*buck_interval + pos_occ;begin++){
        if(begin > file_length){
            break;
        }
        unsigned char pp = fgetc(fp_in);
        if(pp == special_c_temp){
            pp = special_c;
        }
        temp[pp]++;
    }
    unsigned long temp_value = temp[c];
    free(temp);
    return temp_value;
}

unsigned long get_next(unsigned long i){
    for(unsigned long j = i+1;j<128;j++){
        if(c_table[j])
            return j;
    }
    return -1;
}

int search_m(const char* parttern){
    unsigned long i = strlen(parttern)-1;
    unsigned char c =parttern[i];
    unsigned long First = c_table[c];
    unsigned long Last = c_table[get_next(c)]-1;
    while(Last >= First && i > 0){
        c = parttern[i-1];
        First = c_table[c] + get_occ(c, First-1);
        Last = c_table[c] + get_occ(c, Last)-1;
        i = i-1;
    }
    return (int)Last - (int)First + 1;
}

result_arr* search_index(const char* parttern,unsigned long pos_length){
    unsigned long i = strlen(parttern)-1;
    unsigned char c =parttern[i];
    unsigned long First = c_table[c];
    unsigned long Last = c_table[get_next(c)]-1;
    while(Last >= First && i > 0){
        c = parttern[i-1];
        First = c_table[c] + get_occ(c, First-1);
        Last = c_table[c] + get_occ(c, Last)-1;
        i = i-1;
    }
    if(Last< First){
        return NULL;
    }
    trace_t** trace= malloc(sizeof(trace_t)*(Last - First + 1));
    int count = 0;
    int complete_count = 0;
    for(unsigned long i = First;i<Last+1;i++){
        trace[count] =malloc(sizeof(trace_t));
        trace[count]->pos = i;
        trace[count]->pos_c = read_file(i);
        if(trace[count]->pos_c == special_c){
            trace[count]->flag = 1;
            complete_count++;
        }else
            trace[count]->flag = 0;
        count++;
    }
    unsigned long* array =malloc(sizeof(unsigned long)* count);
    while(complete_count < count){
        for(int i =0;i<count;i++){
            if(trace[i]->flag == 0){
                trace[i]->pos =c_table[trace[i]->pos_c] + get_occ(trace[i]->pos_c, trace[i]->pos)-1 ;
                trace[i]->pos_c = read_file(trace[i]->pos);
                if(trace[i]->pos_c == special_c){
                    trace[i]->flag = 1;
                    complete_count++;
                }
            }
        }
    }
    for(int i = 0;i<count;i++){
        unsigned long temp =position[get_occ(trace[i]->pos_c, trace[i]->pos - 1)];
        if(temp == pos_length){
            array[i] = 1;
        }else{
            array[i] = temp+1;
        }
    }
    result_arr* result= malloc(sizeof(result_arr));
    for(int i =0;i<count;i++){
        free(trace[i]);
    }
    free(trace);
    qsort(array,count,sizeof(unsigned long),compare);
    result->array = array;
    result->length = count;
    return result;
}

void search_i(int index,unsigned long pos_length){
    unsigned long pos_index = 0;
    for(unsigned long i=0;i < pos_length;i++){
        if(position[i] == index){
            pos_index = i;
            break;
        }
    }
    unsigned char *result = malloc(sizeof(unsigned char)* 10);
    int times = 1;
    int result_length = 1;
    unsigned long First = c_table[special_c] + pos_index;
    unsigned char temp = read_file(First);
    if(temp ==special_c){
	printf("\n");
	return;
    }
    result[0] = temp;
    while(1){
        First =c_table[temp] + get_occ(temp, First-1);
        temp = read_file(First);
        if(temp == special_c){
            break;
        }
        if(result_length >= times*10-1){
            times++;
            result = realloc(result, times*10*sizeof(unsigned char));
        }
        result[result_length] = temp;
        result_length++;
    }
    for(int i = result_length-1;i>=0;i--){
        printf("%c",result[i]);
    }
    free(result);
    printf("\n");
}

int main(int argc, const char * argv[]) {
    if(! strcmp(argv[1],"\\n")){
        special_c_temp= '\n';
    }else{
        special_c_temp= argv[1][0];
    }
    special_c ='$';
    fp_in = fopen(argv[2],"r");
    char* pos_file_name = malloc((strlen(argv[2])+4)*sizeof(char));
    strcpy(pos_file_name,argv[2]);
    strcat(pos_file_name,".aux");
    FILE *fp_pos =fopen(pos_file_name,"rb");
    unsigned long ht;
    fread(&ht,sizeof(unsigned long),1,fp_pos);
    unsigned long pos_length = ht;
    position = malloc(sizeof(unsigned long)*pos_length);
    fread(position,sizeof(unsigned long),pos_length,fp_pos);
    file_length = get_length_file(fp_in);
    c_table = make_ctable(fp_in);
    initial_occ_table();
    if(! strcmp(argv[4],"-a")){
        result_arr *result = search_index(argv[5],pos_length);
        if(result){
            for(int i =0;i<result->length;i++){
                if(i > 0 && result->array[i-1] == result->array[i]){
                    continue;
                }
                printf("%lu\n",result->array[i]);
            }
            free(result->array);
            free(result);
        }
    }
    if(! strcmp(argv[4],"-m")){
        printf("%d\n",search_m(argv[5]));
    }
    if(! strcmp(argv[4],"-n")){
        result_arr *result = search_index(argv[5],pos_length);
        if(result){
            unsigned long count =0;
            for(int i =0;i<result->length;i++){
                if(i > 0 && result->array[i-1] == result->array[i]){
                    continue;
                }
                count++;
            }
            printf("%lu\n",count);
            free(result->array);
            free(result);
        }
    }
    if(! strcmp(argv[4],"-i")){
        char *index;
        char *a = malloc(sizeof(char)*strlen(argv[4]));
        strcpy(a,argv[5]);
        index = strtok(a," ");
        int times = 0;
        int First = 0;
        int Last = 0;
        while(index != NULL){
            int s_index = 0;
            s_index =atoi(index);
            if(times == 0){
                First = s_index;
            }else{
                Last = s_index;
            }
            times++;
            index = strtok(NULL, " ");
            
        }
        for(int i = First;i<Last+1;i++){
            search_i(i,pos_length);
        }
    }
    fclose(fp_pos);
    if(fp_in != NULL)
	fclose(fp_in);
    free(position);
    unsigned long count = file_length / buck_interval;
    for(int i =1;i<count+1;i++){
        free(occ_table[i]);
    }
    free(occ_table);
    return 0;

}
