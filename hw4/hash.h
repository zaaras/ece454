
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <pthread.h>
#include <vector>
#include "list.h"

#define HASH_INDEX(_addr,_size_mask) (((_addr) >> 2) & (_size_mask))

#if defined(LLL) || defined(ELL)
  static pthread_mutex_t *locks;
#endif

template<class Ele, class Keytype> class hash;

template<class Ele, class Keytype> class hash {
 private:
  unsigned my_size_log;
  unsigned my_size;
  unsigned my_size_mask;
  list<Ele,Keytype> *entries;
  list<Ele,Keytype> *get_list(unsigned the_idx);

 public:
  void setup(unsigned the_size_log=5);
  void insert(Ele *e);
  inline Ele *lookup(Keytype the_key);
  void print(FILE *f=stdout);
  void reset();
  void cleanup();

#if defined(LLL) || defined(ELL)
  inline Ele* lookupInsert(Keytype fkey);
  int testLock(Keytype the_key);
  int lockList(Keytype the_key);
  void unlockList(Keytype the_key);
#endif 
};

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::setup(unsigned the_size_log){
  my_size_log = the_size_log;
  my_size = 1 << my_size_log;
  my_size_mask = (1 << my_size_log) - 1;
  entries = new list<Ele,Keytype>[my_size];

#if defined(LLL) || defined(ELL)
  size_t i;
  //std::vector<pthread_mutex_t>::iterator it;
  //it = locks.begin();
  locks = new pthread_mutex_t[my_size];
  for(i=0;i<my_size;i++){
    if (pthread_mutex_init(&locks[i], NULL) != 0){
      printf("\n mutex init failed\n");
    }
  }

#endif

}

#if defined(LLL) || defined(ELL)
template<class Ele, class Keytype> 
int
hash<Ele,Keytype>::lockList(Keytype the_key){ 
  return pthread_mutex_lock(&locks[HASH_INDEX(the_key,my_size_mask)]);
}

template<class Ele, class Keytype> 
int
hash<Ele,Keytype>::testLock(Keytype the_key){ 
  return pthread_mutex_trylock(&locks[HASH_INDEX(the_key,my_size_mask)]);
}

template<class Ele, class Keytype> 
void
hash<Ele,Keytype>::unlockList(Keytype the_key){ 
  pthread_mutex_unlock(&locks[HASH_INDEX(the_key,my_size_mask)]);
} 

template<class Ele, class Keytype>
inline
Ele*
hash<Ele,Keytype>::lookupInsert(Keytype fkey){
	Ele *s;
	 list<Ele,Keytype> *l;

  //pthread_mutex_lock(&locks[HASH_INDEX(the_key,my_size_mask)]);

  l = &entries[HASH_INDEX(fkey,my_size_mask)];

  if (!(s = l->lookup(fkey))){
	  s = new Ele(fkey);
  	  entries[HASH_INDEX(fkey,my_size_mask)].push(s);
  }

  return s;
  //pthread_mutex_unlock(&locks[HASH_INDEX(the_key,my_size_mask)]);

}
#endif

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
inline
Ele *
hash<Ele,Keytype>::lookup(Keytype the_key){
  list<Ele,Keytype> *l;  
  l = &entries[HASH_INDEX(the_key,my_size_mask)];
  return  l->lookup(the_key);
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

  entries[HASH_INDEX(e->key(),my_size_mask)].push(e);

}

#endif
