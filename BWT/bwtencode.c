#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct bucket {
    unsigned long* postion_array;
    unsigned long limit;
    unsigned long length;
}bucket;
unsigned char* content;
unsigned long file_length;
unsigned char special_c_temp;
unsigned char special_c;

bucket* make_bucket(){
    bucket* a = malloc(sizeof(bucket));
    a->limit =16;
    a->length = 0;
    a->postion_array = malloc(sizeof(unsigned long)* a->limit);
    return a;
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

unsigned char* read_file(FILE *fp,unsigned long length){
    unsigned char *content = malloc(sizeof(unsigned char) *length);
    rewind(fp);
    int i = 0;
    unsigned char c;
    while(1){
        c =fgetc(fp);
        if(c == special_c_temp){
            content[i] = special_c;
        }else{
            content[i] = c;
        }
        if(feof(fp) && c !='\n'){
            break;
        }
        i++;
    }
    return content;
}
bucket* extend_bucket(bucket* a){
    a->postion_array = realloc(a->postion_array,(a->limit+16)* sizeof(unsigned long));
    a->limit+=16;
    return a;
}

bucket** input_bucket(unsigned char* input,unsigned long length){
    bucket** result = malloc(sizeof(bucket*)*128);
    for(int i= 0;i<128;i++){
        result[i] = 0;
    }
    for(unsigned long i = 0;i<length;i++){
        if(! result[input[i]]){
            result[input[i]] = make_bucket();
        }
        if(result[input[i]]->length >= result[input[i]]->limit -1){
            result[input[i]] = extend_bucket(result[input[i]]);
        }
        result[input[i]]->postion_array[result[input[i]]->length] = i;
        result[input[i]]->length++;
    }
    return result;
}

int cmp(const void *a,const void *b){
    unsigned long l = *(unsigned long*) a;
    unsigned long m = *(unsigned long*) b;
    for(int i = 0;i<file_length;i++){
        if(content[l+i] < content[m+i]){
            return -1;
        }
        if(content[l+i] > content[m+i]){
            return 1;
        }
        if(l+i >= file_length-1){
            l = l-file_length;
        }
        if(m+i >= file_length-1){
            m = m-file_length;
        }
    }
    return 0;
}

void sort_each_bucket(bucket **a){
    for(int i = 0;i<128;i++){
        if(a[i] && a[i]->length > 1){
            qsort(a[i]->postion_array,a[i]->length,sizeof(unsigned long),cmp);
        }
    }
}

int main(int argc, const char * argv[]) {
    FILE *fp_in = fopen(argv[3],"r");
    FILE *fp_out = fopen(argv[4],"w");
    char* pos_file_name = malloc((strlen(argv[4]) + 4)*sizeof(char));
    strcpy(pos_file_name,argv[4]);
    char p[] = ".aux";
    FILE *fp_positon = fopen(strcat(pos_file_name,p),"wb");
    if(! strcmp(argv[1],"\\n")){
        special_c_temp= '\n';
    }else{
        special_c_temp= argv[1][0];
    }
    special_c = '$';
    file_length = get_length_file(fp_in);
    content = read_file(fp_in, file_length);
    bucket** result = input_bucket(content, file_length);
    sort_each_bucket(result);
    for(int i = 0;i<128;i++){
        if(! result[i]){
            continue;
        }
        for(int j =0;j<result[i]->length;j++){
            if(result[i]->postion_array[j] == 0){
                if(content[file_length-1] == special_c){
                    fputc(special_c_temp, fp_out);
                }else{
                    fputc(content[file_length-1], fp_out);
                }
            }else{
                if(content[result[i]->postion_array[j] - 1] == special_c){
                    fputc(special_c_temp, fp_out);
                }else{
                    fputc(content[result[i]->postion_array[j] - 1], fp_out);
                }
            }
        }
    }
    fwrite(&result[special_c]->length,sizeof(unsigned long),1,fp_positon);
    unsigned long count = 1;
    unsigned long *postion = malloc(sizeof(unsigned long)*(result[special_c]->length));
    for(unsigned long i=0;i<file_length;i++){
        if(content[i] ==special_c){
            for(unsigned long j = 0;j<result[special_c]->length;j++){
                if(result[special_c]->postion_array[j] == i){
                    postion[j] = count;
                    count++;
                }
            }
        }
    }
    fwrite(postion,sizeof(unsigned long),result[special_c]->length,fp_positon);
    for(int i = 0;i<128;i++){
        if(result[i]){
            free(result[i]->postion_array);
        }
    }
    free(content);
    free(result);
    fclose(fp_in);
    fclose(fp_out);
    fclose(fp_positon);
    free(postion);
    return 0;
}
