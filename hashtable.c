#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define CAP 65515

int parseint(FILE *s) {
  unsigned int c, n;
  if((n = fgetc(s) - '0') == '\0') return 0;
  while((c = fgetc(s)) >= '0')
    n = 10*n + c - '0';
  return n;
}

struct movie {
  char *title;
  char *director;
  char *genre;
  int  year;
};
struct movie *new_movie(char *t, char *d, char *g, int year) {
  struct movie *m = (struct movie*)malloc(sizeof(struct movie));
  m->title = (char*)malloc(strlen(t) + 1);
  m->director = (char*)malloc(strlen(d) + 1);
  m->genre = (char*)malloc(strlen(g) + 1);
  strcpy(m->title, t);
  strcpy(m->director, d);
  strcpy(m->genre, g);
  m->year = year;
  return m;
}
void free_movie(struct movie *m) {
  free(m->title); free(m->director); free(m->genre); free(m);
}
typedef struct movie Movie;

unsigned long hash(char *s) {
  unsigned long i = 0;
  for(int x = 0; s[x]; x++)
    i += s[x];
  return i % CAP; }

struct item {
  char *key; 
  Movie *data;
};
typedef struct item Item;
struct table {
  Item **data;
  char *name;
  int size, count; };
typedef struct table Table;

Item *new_item(char *key, Movie *m) {
  Item *item = (Item*)malloc(sizeof(Item));
  item->key = (char*)malloc(strlen(key) + 1);
  item->data = new_movie(m->title, m->director, m->genre, m->year);
  strcpy(item->key, key);
  return item;
}
void free_item(Item *i) {
  free(i->key); free_movie(i->data); free(i); }


Table *new_table(int size, char *name) {
  if(mkdir("data", 0777) != 1) { }
  chdir("data");
  if(mkdir(name, 0777) != -1) {}
  chdir("..");
  Table *t = (Table*)malloc(sizeof(Table));
  t->name = (char*)malloc(strlen(name) + 1);
  strcpy(t->name, name);
  t->size = size;
  t->count = 0;
  t->data = (Item**)calloc(t->size, sizeof(Item*));
  for(int i = 0; i < t->size; i++)
    t->data[i] = NULL;
  return t;
}
void free_table(Table* t) {
  for(int i = 0; i < t->size; i++) {
    Item *item = t->data[i];
    if(item != NULL) free_item(item);
  }
  free_item(*t->data);
  free(t->name);
  free(t);
}

void insert(Table *t, char *key, Movie *m) {
  Item *i = new_item(key, m);
  unsigned long x = hash(key);
  Item *j = t->data[x];
  if(j == NULL) {
    if(t->count == t->size) {
      printf("Insert Error\n");
      free_item(i);
      return;
    }
    t->data[x] = i;
    t->count++;
  }
  else {
    if(strcmp(j->key, key) == 0) {
      t->data[x]->data = m;
      return;
    } else {
      printf("Collision Error\n");
      // fuck
      return;
    }
  }
}

void s_insert(Table *t, char *key, Movie *m) {
  insert(t, key, m);
  chdir("data");
  chdir(t->name);
  char fp[255] = "";
  strcat(fp, m->title);
  strcat(fp, ".txt");

  printf("%s", fp);
  FILE *ptr = fopen(fp, "w");
  fprintf(ptr, "%s\n", m->title);
  fprintf(ptr, "%s\n", m->director);
  fprintf(ptr, "%s\n", m->genre);
  fprintf(ptr, "%d\n", m->year);
  fclose(ptr);
  chdir("../..");
}

Movie *search(Table *t, char *key) {
  int x = hash(key);
  Item *i = t->data[x];
  if(i != NULL) {
    if(strcmp(i->key, key) == 0)
      return i->data;
  }
  return NULL;
}

void delete(Table *t, char *key) {
  char fp[255];
  int x = hash(key);
  Item *i = t->data[x];
  strcpy(fp, i->data->title);
  strcat(fp, ".txt");
  chdir("data");
  chdir("d.Table");
  remove(fp);
  if(i != NULL) free_item(i);;
  chdir("../..");
  t->data[x] = NULL; 
}

void print(Table *t) {
  for(int i = 0; i < t->size; i++)
    if(t->data[i] != NULL)
      printf("[%d] : %s, %s\n", i, t->data[i]->key, t->data[i]->data->title);
}

char *strrem(char *s, const char *sub) {
  int len = strlen(sub);
  if(len > 0) {
    char *p = s;
    while((p = strstr(p, sub)) != NULL)
      memmove(p, p + len, strlen(p + len) + 1);
  }
  return s;
}

const char *get_ext(char *file) {
  const char *ext = strrchr(file, '.');
  if(!ext) return NULL;
  else return ext + 1;
}

Table *read_dir(char *dp) {
  Table *t = new_table(CAP, dp);
  DIR *d;
  char **farr = (char**)malloc(sizeof(char) * 255 * CAP);
  int i = 0;
  struct dirent *dir;
  chdir("data");
  d = opendir("d.Table");
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if(strcmp(get_ext(dir->d_name), "txt") == 0) {
        farr[i++] = dir->d_name;
  }}}
  closedir(d);
  chdir("d.Table");
  for(int y = 0; y < i; y++) {
    if(farr[y] != NULL) {
      FILE *file = fopen(farr[y], "r");
      char a[255];
      char b[255];
      char c[255];
      char f[255];
      Movie m;
      fscanf(file, "%s\n", a);
      m.title = a;
      fscanf(file, "%s\n", b); 
      m.director = b;
      fscanf(file, "%s\n", c);
      m.genre = c;
      fscanf(file, "%s\n", f);
      m.year = atoi(f);
      insert(t, m.title, &m);
    }
  }
  free(farr);
  chdir("../..");
  return t;
}

int main(int argc, char *argv[]) {
  Table *t = read_dir("d.Table");
  if(argc < 2) {
    printf("No expression given.\n");
    return 1;
  }
  if(strcmp(argv[1], "INSERT") == 0) {
    if(argc < 6) { printf("Too few argument was given!\n"); return 1; }
    if(argc > 6) { printf("Too many argument was given!\n"); return 1; }
    Movie m;
    m.title     = argv[2];
    m.director  = argv[3];
    m.genre     = argv[4];
    m.year      = atoi(argv[5]);
    s_insert(t, m.title, &m);
  }
  else if(strcmp(argv[1], "SELECT") == 0) {
    Movie *m = search(t, argv[2]);
    if(m == NULL) { printf("No such movie was found!\n"); return 1; }
    printf("TITLE\t\t%s\n", m->title); 
    printf("DIRECTOR\t%s\n", m->director); 
    printf("GENRE\t\t%s\n", m->genre); 
    printf("YEAR\t\t%d\n", m->year); 
  }
  else if(strcmp(argv[1], "DELETE") == 0) {
    Movie *m = search(t, argv[2]);
    if(m == NULL) { printf("No such movie was found!\n"); return 1; }
    delete(t, argv[2]);
  }
  else if(strcmp(argv[1], "LIST") == 0) {
    print(t);
    //Movie *m = search(t, "titanic");
    //printf("%s\n", m->title);
    //for(int i = 0; i < CAP; i++)
       // if(t->data[i] != NULL) printf("%s\n",t->data[i]->data->title);
  }
  else printf("Argument not recognized.\n");
  return 0;
}
