#ifndef _DATAPOOL_H_
#define _DATAPOOL_H_

#include "window.h"
#include "item_type.h"
#include "stringfunc.h"
#include "rng.h"
#include <string>
#include <istream>
#include <list>
#include <map>
#include <fstream>

/* Important notes:
 * All classes to be used with Data_pool must have the following functions:
 *   void assign_uid(int)           - Assign its UID (it's not required to
 *                                    actually STORE this, but the function must
 *                                    exist nevertheless).
 *   bool load_data(std::istream&)  - Load data from a stream. Should return
 *                                    false on failure, true on success.
 *   std::string get_data_name()    - Return its formal name
 */

// Everything is inline cause compilers are dumb etc. etc.

template <class T>
struct Data_pool
{
public:
  Data_pool()
  {
    next_uid = 0;
  }

  virtual ~Data_pool()
  {
    for (typename std::list<T*>::iterator it = instances.begin();
         it != instances.end();
         it++) {
      if (*it) {
        delete (*it);
      }
    }
  }

  bool load_from(std::string filename)
  {
    std::ifstream fin;
    fin.open(filename.c_str());
    if (!fin.is_open()) {
      debugmsg("Failed to open '%s'", filename.c_str());
      return false;
    }

    while (!fin.eof()) {
      if (!load_element(fin, filename)) {
        return false;
      }
    }
    return true;
  }

  bool load_element(std::istream &data, std::string filename)
  {
    T* tmp = new T;
    if (!tmp->load_data(data)) {
      return false;
    }
    return add_element(tmp, filename);
  }

  bool add_element(T* tmp, std::string filename = "no file")
  {
    std::string name = no_caps( trim( tmp->get_data_name() ) );
    if (name.empty()) {
      debugmsg("Data_pool member with no name! (%s)", filename.c_str());
      return false;
    } else if (name_map.count(name) > 0) {
      debugmsg("Data_pool member with duplicate name '%s'. (%s)",
               name.c_str(), filename.c_str());
      return false;
    }
    tmp->assign_uid(next_uid);
    instances.push_back(tmp);
    name_map[ name     ] = tmp;
    uid_map [ next_uid ] = tmp;
    next_uid++;
    return true;
  }

  T* lookup_uid(int uid)
  {
    if (uid_map.count(uid) == 0) {
      return NULL;
    }
  
    return uid_map[uid];
  }

  T* lookup_name(std::string name)
  {
    name = no_caps(name);
    name = trim(name);
    if (name_map.count(name) == 0) {
      return NULL;
    }
  
    return name_map[name];
  }

// Return the first result that partially matches name
  T* lookup_partial_name(std::string name)
  {
    name = no_caps(name);
    for (typename std::list<T*>::iterator it = instances.begin();
         it != instances.end();
         it++) {
      std::string item_name = no_caps( (*it)->get_name() );
      if (item_name.find(name) != std::string::npos) {
        return *it;
      }
    }
    return NULL;
  }

  T* random_instance()
  {
    if (size() == 0) {
      return NULL;
    }
    int roll = rng(0, size() - 1);
    typename std::list<T*>::iterator it = instances.begin();
    for (int i = 0; i < roll; i++) {
      it++;
    }
    return *it;
  }

  int size()
  {
    return instances.size();
  }

// We have a single public data member.
  std::list<T*> instances;

private:
  std::map<int,T*> uid_map;
  std::map<std::string,T*> name_map;
  int next_uid;
};

template <>
inline
Data_pool<Item_type>::~Data_pool()
{
  std::list<Item_type*>::iterator it = instances.begin();
  while (it != instances.end()) {
    if (*it) {
      delete (*it);
    }
    it = instances.erase(it);
  }
}

template <>
inline
bool Data_pool<Item_type>::load_element(std::istream &data,
                                        std::string filename)
{
  Item_type* tmp;
  std::string item_category;
  data >> item_category;
  item_category = no_caps(item_category);
  item_category = trim(item_category);

  if (item_category == "weapon" || item_category == "vanilla") {
    tmp = new Item_type;

  } else if (item_category == "armor" || item_category == "armour" ||
             item_category == "clothing") {
    tmp = new Item_type_clothing;

  } else if (item_category == "ammo" || item_category == "ammunition") {
    tmp = new Item_type_ammo;

  } else if (item_category == "gun" || item_category == "firearm" ||
             item_category == "launcher") {
    tmp = new Item_type_launcher;

  } else if (item_category == "food" || item_category == "drink") {
    tmp = new Item_type_food;

  } else if (item_category == "tool") {
    tmp = new Item_type_tool;

  } else if (item_category == "book") {
    tmp = new Item_type_book;

  } else if (item_category == "container") {
    tmp = new Item_type_container;

  } else if (item_category == "corpse") {
    tmp = new Item_type_corpse;

  } else if (item_category == "#") {  // It's a commented line - skip it
    std::string junk;
    std::getline(data, junk);
    return load_element(data, filename);

  } else if (item_category.empty()) {
    return false;

  } else {
    debugmsg("Unknown item category '%s' (%s)", item_category.c_str(),
             filename.c_str());
    return false;
  }
  if (!tmp->load_data(data)) {
    return false;
  }
  tmp->assign_uid(next_uid);
  instances.push_back(tmp);
  uid_map[next_uid] = tmp;
  name_map[no_caps(tmp->get_data_name())] = tmp;
  next_uid++;
  return true;
};
#endif
