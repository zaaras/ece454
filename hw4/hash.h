
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <pthread.h>
#include <vector>
#include "list.h"

#define HASH_INDEX(_addr,_size_mask) (((_addr) >> 2) & (_size_mask))

template<class Ele, class Keytype> class hash;

template<class Ele, class Keytype> class hash {
 private:
  unsigned my_size_log;
  unsigned my_size;
  unsigned my_size_mask;
  list<Ele,Keytype> *entries;
  list<Ele,Keytype> *get_list(unsigned the_idx);
#ifdef LLL
  std::vector<pthread_mutex_t> locks;
  pthread_mutex_t lock;
#endif

 public:
  void setup(unsigned the_size_log=5);
  void insert(Ele *e);
  Ele *lookup(Keytype the_key);
  void print(FILE *f=stdout);
  void reset();
  void cleanup();
};

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::setup(unsigned the_size_log){
  my_size_log = the_size_log;
  my_size = 1 << my_size_log;
  my_size_mask = (1 << my_size_log) - 1;
  entries = new list<Ele,Keytype>[my_size];

#ifdef LLL
  size_t i;
  std::vector<pthread_mutex_t>::iterator it;
  it = locks.begin();

  for(i=0;i<my_size;i++){
    if (pthread_mutex_init(&lock, NULL) != 0){
      printf("\n mutex init failed\n");
    }else{
      it = locks.insert(it, lock);
    }

  }
#endif

}

template<class Ele, class Keytype> 
list<Ele,Keytype> *
hash<Ele,Keytype>::get_list(unsigned the_idx){
  if (the_idx >= my_size){
    fprintf(stderr,"hash<Ele,Keytype>::list() public idx out of range!\n");
    exit (1);
  }
  return &entries[the_idx];
}

template<class Ele, class Keytype> 
Ele *
hash<Ele,Keytype>::lookup(Keytype the_key){
  list<Ele,Keytype> *l;
  Ele *  tmp;
#ifdef LLL
    int err=pthread_mutex_lock(&locks[HASH_INDEX(the_key,my_size_mask)]);
  if(err!=0){
    printf("Problem : %d\n",err);
    exit(1);
  }
#endif
  l = &entries[HASH_INDEX(the_key,my_size_mask)];
  tmp =  l->lookup(the_key);
#ifdef LLL
  pthread_mutex_unlock(&locks[HASH_INDEX(the_key,my_size_mask)]);
#endif
  return tmp;
}  

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::print(FILE *f){
  unsigned i;

  for (i=0;i<my_size;i++){
    entries[i].print(f);
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::reset(){
  unsigned i;
  for (i=0;i<my_size;i++){
    entries[i].cleanup();
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::cleanup(){
  unsigned i;
  reset();
  delete [] entries;
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::insert(Ele *e){
#ifdef LLL
  int err=pthread_mutex_lock(&locks[HASH_INDEX(e->key(),my_size_mask)]);
  if(err!=0){
    printf("Problem : %d\n",err);
    exit(1);
  }
#endif
  entries[HASH_INDEX(e->key(),my_size_mask)].push(e);
#ifdef LLL
  pthread_mutex_unlock(&locks[HASH_INDEX(e->key(),my_size_mask)]);
#endif
}


#endif
