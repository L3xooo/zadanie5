#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// TODO
// Vase vlastne pomocne makra, datove typy a funkcie.

WAREHOUSE WH_filter(char warehouse_filter,char name[],double lat,double lon){
    char word[MAX_NAME];
    WAREHOUSE WH_in_use;
    int capacity = 0,count = 0;
    double file_lat = 0,file_lon = 0,d = 0,max_d = 0;
    GPS gps1 = {lat,lon};
    FILE *scan;
    scan = fopen(WAREHOUSE_DB_FILE,"r");
    while(fscanf(scan,"%s %lf %lf %d",word,&file_lat,&file_lon,&capacity)!= EOF){
        count++;
        if(warehouse_filter == 'w'){
            if(strcmp(word,name) == 0){
                WH_in_use.capacity = capacity;
                WH_in_use.gps.lon = file_lon;
                WH_in_use.gps.lat = file_lat;
                strcpy(WH_in_use.name,word);
                break;
            }
        }
        else if(warehouse_filter == 'e' || warehouse_filter == 'n'){
            GPS gps2 = {file_lat,file_lon};
            d = distance(gps1,gps2);
            if (count == 1){
                max_d = d;
                WH_in_use.capacity = capacity;
                WH_in_use.gps = gps2;
                strcpy(WH_in_use.name,word);
            }
            if(d<max_d){
                max_d = d;
                WH_in_use.capacity = capacity;
                WH_in_use.gps = gps2;
                strcpy(WH_in_use.name,word);
            }
        }
    }
    fclose(scan);
    return WH_in_use;
}

void ITM_filter(char item_sort,WAREHOUSE db,int count){
    ITEM tmp;
    int i = 0,j = 0;
    if(item_sort == 'a'){
        for(i = 0;i<count-1;i++){
            for(j = 0;j<count-1-i;j++){             //Bubble sort Geeksforgeeks
                if(db.items[j].price>db.items[j+1].price){
                    tmp = db.items[j];
                    db.items[j] = db.items[j+1];
                    db.items[j+1] = tmp;
                }
            }
        }
    }
    else if(item_sort == 'd'){
        for(i = 0;i<count-1;i++){
            for(j = 0;j<count-1-i;j++){
                if(db.items[j].price<db.items[j+1].price){
                    tmp = db.items[j];
                    db.items[j] = db.items[j+1];
                    db.items[j+1] = tmp;
                }
            }
        }
    }
    else{
        for(i = 0;i<count-1;i++){
            for(j = 0;j<count-1-i;j++){
                if(strcmp(db.items[j].name,db.items[j+1].name) > 0){
                //if(db.items[j].name[0]>db.items[j+1].name[0]){
                    tmp = db.items[j];
                    db.items[j] = db.items[j+1];
                    db.items[j+1] = tmp;
                }
            }
        }
    }
}

int main(int argc, char *argv[]){
    WAREHOUSE WH_in_use;
    int i = 0, opt = 0,capacity = 0,count = 0,price = 0,counter = 0,break_option = 0;
    double d_lat = 0,d_lon = 0,file_lat = 0,file_lon = 0;
    char word[MAX_NAME],item_name[MAX_NAME],price_char[10];
    char *buffer;
    WAREHOUSE *db;
    ITEM *item_array;
    char warehouse_filter = ' ',item_sort = ' ';
    char* name,*lat,*lon;
    char* optstring = ":w:n:e:ad";
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'w':
                warehouse_filter = 'w';
                name = optarg;
                break;
            case 'n':
                if (warehouse_filter != 'e'){
                    warehouse_filter = 'n';
                }
                lat = optarg;
                break;
            case 'e':
                if (warehouse_filter != 'n'){
                    warehouse_filter = 'e';
                }
                lon = optarg;
                break;
            case 'a':
                item_sort = 'a';
                break;
            case 'd':
                item_sort = 'd';
                break;
            default:
                return 1;
        }
    }

    if(warehouse_filter == 'e' || warehouse_filter == 'n'){
        d_lat = atof(lat);
        d_lon = atof(lon);
    }
    FILE *warehouse_file,*items_file;
    warehouse_file = fopen(WAREHOUSE_DB_FILE,"r");
    db = (WAREHOUSE*) malloc(sizeof(WAREHOUSE)*1);
    while(fscanf(warehouse_file,"%s %lf %lf %d",word,&file_lat,&file_lon,&capacity) != EOF){
        fprintf(stdout,"%s %.3lf %.3lf %d\n",word,file_lat,file_lon,capacity);
        count++;
        if(warehouse_filter == ' '){
            db = (WAREHOUSE*) realloc(db,count * sizeof(WAREHOUSE));
            db[count-1].capacity = capacity;
            db[count-1].gps.lat = file_lat;
            db[count-1].gps.lon = file_lon;
            strcpy(db[count-1].name,word);
        }
    }
    fclose(warehouse_file);

    if(warehouse_filter != ' '){
        count = 1;
        WH_in_use = WH_filter(warehouse_filter,name,d_lat,d_lon);
        db[0] = WH_in_use;
    }


    for(i = 0;i<count;i++) {
        break_option = 0;
        counter = 0;
        buffer = (char*) malloc(50*sizeof(char));
        item_array = (ITEM*) malloc(1*sizeof(ITEM));
        sprintf(buffer, "%s%s%s.txt", ITEMS_FOLDER, PATH_SEPARATOR, db[i].name);
        items_file = fopen(buffer, "r");
        free(buffer);

        if (items_file == NULL) {                                            //test na chybu 1
            fprintf(stderr, "FILE_ERROR %s.txt\n", db[i].name);
            free(item_array);
            continue;
        }

        while (fscanf(items_file, "%s %s", item_name, price_char) != EOF) { //test na chybu 2

            counter++;

            if (counter > db[i].capacity) {
                fprintf(stderr, "CAPACITY_ERROR %s.txt\n", db[i].name);
                break_option = 1;
                break;
            }

            //if (item_name == "" || price_char == "") {                            // test na chyba 3
            if(strcmp(item_name,"") == 0 || strcmp(price_char,"") == 0){
                fprintf(stderr, "FORMAT_ERROR %s.txt\n", db[i].name);
                break_option = 1;
                break;
            }

            for (size_t k = 0; k < strlen(item_name); k++) {
                if (isalpha(item_name[k]) == 0) {
                    fprintf(stderr, "FORMAT_ERROR %s.txt\n", db[i].name);
                    break_option = 1;
                    break;
                }
            }
            if (break_option == 1){
                break;
            }

            for (size_t k = 0; k < strlen(price_char); k++) {
                if (isdigit(price_char[k]) == 0) {
                    fprintf(stderr, "FORMAT_ERROR %s.txt\n", db[i].name);
                    break_option = 1;
                    break;
                }
            }
            if (break_option == 1){
                break;
            }

            //TESTY struktur

            price = atoi(price_char);

            item_array = (ITEM*)realloc(item_array, counter * sizeof(ITEM));
            item_array[counter - 1].price = price;
            strcpy(item_array[counter - 1].name, item_name);
            //printf("%s %d\n",item_array[counter-1].name,item_array[counter-1].price);
        }

        if (break_option == 1){
            fclose(items_file);
            free(item_array);
            continue;
        }

        db[i].items = item_array; //ulozi item do struktury

        ITM_filter(item_sort,db[i],counter);  // filter na vypis

        fprintf(stdout,"%s %.3lf %.3lf %d :\n",db[i].name,db[i].gps.lat,db[i].gps.lon,db[i].capacity);
        for(int x = 0;x<counter;x++)
            fprintf(stdout,"%d. %s %d\n",x+1,db[i].items[x].name,db[i].items[x].price);

        fclose(items_file);
        free(item_array);

    } 
    free(db);
    return 0;
}
