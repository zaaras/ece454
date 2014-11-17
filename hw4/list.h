
#ifndef LIST_H
#define LIST_H

#include <stdio.h>

#ifdef ELL
#include <pthread.h>
#include <vector>
#endif

template<class Ele, class Keytype> class list;

template<class Ele, class Keytype> class list {
 private:
  Ele *my_head;
  unsigned long long my_num_ele;
 public:
  list(){
    my_head = NULL;
    my_num_ele = 0;
  }

  void setup();

  unsigned num_ele(){return my_num_ele;}

  Ele *head(){ return my_head; }
  Ele *lookup(Keytype the_key);

  void push(Ele *e);
  Ele *pop();
  void print(FILE *f=stdout);

  void cleanup();

#ifdef ELL
  std::vector<pthread_mutex_t> locks;
  pthread_mutex_t lock;

  void lockElement(Keytype the_key);
  void unlockElement(Keytype the_key);
#endif
};

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::setup(){
  my_head = NULL;
  my_num_ele = 0;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::push(Ele *e){
  e->next = my_head;
  my_head = e;
  my_num_ele++;

#ifdef ELL
    std::vector<pthread_mutex_t>::iterator it;
    it = locks.begin();
    if (pthread_mutex_init(&lock, NULL) != 0){
      printf("\n mutex init failed\n");
    }else{
      it = locks.insert(it, lock);
    }
#endif
}

#ifdef ELL
template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::lockElement(Keytype the_key){
  Ele *e_tmp = my_head;
  size_t lockCount = 0;

  while (e_tmp && (e_tmp->key() != the_key)){
    e_tmp = e_tmp->next;
    lockCount++;
  }
  pthread_mutex_unlock(&locks[lockCount]);

}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::unlockElement(Keytype the_key){
  Ele *e_tmp = my_head;
  size_t lockCount = 0;

  while (e_tmp && (e_tmp->key() != the_key)){
    e_tmp = e_tmp->next;
    lockCount++;
  }
  pthread_mutex_lock(&locks[lockCount]);

}
#endif

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::pop(){
  Ele *e;
  e = my_head;
  if (e){
    my_head = e->next;
    my_num_ele--;
  }
  return e;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::print(FILE *f){
  Ele *e_tmp = my_head;

  while (e_tmp){
    e_tmp->print(f);
    e_tmp = e_tmp->next;
  }
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::lookup(Keytype the_key){
  Ele *e_tmp = my_head;

  while (e_tmp && (e_tmp->key() != the_key)){
    e_tmp = e_tmp->next;
  }
  return e_tmp;
}

template<class Ele, class Keytype> 
void
list<Ele,Keytype>::cleanup(){
  Ele *e_tmp = my_head;
  Ele *e_next;

  while (e_tmp){
    e_next = e_tmp->next;
    delete e_tmp;
    e_tmp = e_next;
  }
  my_head = NULL;
  my_num_ele = 0;
}

#endif
