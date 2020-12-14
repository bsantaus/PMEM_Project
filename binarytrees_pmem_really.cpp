/* The Computer Language Benchmarks Game
* https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
*
* Contributed by Jon Harrop
* Modified by Alex Mizrahi
* Modified by Andreas Sch�fer
* Modified by aardsplat-guest
*  *reset*
*/

#include <iostream>
#include <fstream>
#include <algorithm>
#include <future>
#include <vector>

#include <libpmemobj/atomic_base.h>
#include <boost/pool/object_pool.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/utils.hpp>
#include <unistd.h>
#define CREATE_MODE_RW (S_IWUSR | S_IRUSR)





constexpr int threads{16};
const char *LAYOUT = "btreez_cpp";


class Node {
   public:
      pmem::obj::persistent_ptr<Node> l,r;
      Node() : l(0),r(0) {}
      Node(pmem::obj::persistent_ptr<Node> l,pmem::obj::persistent_ptr<Node> r) : l(l),r(r) {}
      int check() const
      {
         if (l)
            return l->check() + 1 + r->check();
         else return 1;
      }
      void make(int d)
      {
         std::cout << "entering transaction" << std::endl;
         auto pool = pmem::obj::pool_by_vptr(this);
         pmem::obj::transaction::run(pool, [this, d] {
            this->l = pmem::obj::make_persistent<Node>();
            this->r = pmem::obj::make_persistent<Node>();
            if (d>=1) {
               this->l->make(d-1);
               this->r->make(d-1);
            }
         });
         if (this->l == NULL)
            std::cout << "yeet?????" << std::endl;
         return;
      }
      void remove()
      {
         std::cout << "entering transaction" << std::endl;
         auto pool = pmem::obj::pool_by_vptr(this);
         pmem::obj::transaction::run(pool, [this] {
            std::cout << "in transaction?" << std::endl;
            if (this->l != NULL) {
               std::cout << "delyeeting children" << std::endl;
               this->l->remove();
               this->r->remove();
               pmem::obj::delete_persistent<Node>(this->l);
               pmem::obj::delete_persistent<Node>(this->r);
               this->l = NULL;
               this->r = NULL;
            }
            if (this->l == NULL) std::cout << "wack" << std::endl;
         });
         std::cout << "delyeeet?????" << std::endl;
         return;
      }
};

struct root {
   pmem::obj::persistent_ptr<Node> rootp;
};

pmem::obj::pool<root> pop;



// int make_iteration(int from,int to,int d,bool thread)
// {
//    int c{0};
//    if (thread) {
//       std::vector<std::future<int>>futures{};
//       for (int j=0; j<threads; ++j) {
//          int span{(to-from+1)/threads};
//          futures.emplace_back(std::async(std::launch::async,
//             make_iteration,from+span*j,span+span*j,d,false));
//       }
//       for (auto& fti : futures) {
//          c += fti.get();
//       }
//    }
//    else {
//       for (int i=from; i<=to; ++i) {
//          pmem::obj::persistent_ptr<Node> a = make(d);
//          c += a->check();
//       }
//    }
//    return c;
// }

bool file_exists(const std::string &path) {
   std::ifstream f(path.c_str());
   return f.good();
}

int main(int argc,char *argv[])
{
   std::string path = "bintrees_cpp";

   if (file_exists(path)) {
      std::cout << "opening existing pmem file\n";
      pop = pmem::obj::pool<root>::open(path, LAYOUT);
   } else {
      std::cout << "opening new pmem file\n";
      pop = pmem::obj::pool<root>::create(path, LAYOUT, 1024*1024*64, CREATE_MODE_RW);
   
      pmem::obj::make_persistent_atomic<Node>(pop, pop.root()->rootp);
   }


   auto root_node = pop.root()->rootp;

   root_node->make(2);
   root_node->remove();
   return 0;  
   
   // int min_depth = 4,
   //    max_depth = std::max(min_depth+2,
   //       (argc == 2 ? atoi(argv[1]) : 10)),
   //    stretch_depth = max_depth+1;

   // {
   //    pmem::obj::persistent_ptr<Node> c = make(stretch_depth);
   //    std::cout << "hey" << std::endl;
   //    return 0;
   //    std::cout << "stretch tree of depth " << stretch_depth << "\t "
   //       << "check: " << c->check() << std::endl;
   // }

   // pmem::obj::persistent_ptr<Node> long_lived_tree=make(max_depth);

   // for (int d=min_depth; d<=max_depth; d+=2) {
   //    int iterations = 1 << (max_depth - d + min_depth);
   //    int   c=0;
   //    c = make_iteration(1,iterations,d,true);
   //    std::cout << iterations << "\t trees of depth " << d << "\t "
   //       << "check: " << c << std::endl;
   // }

   // std::cout << "long lived tree of depth " << max_depth << "\t "
   //    << "check: " << (long_lived_tree->check()) << "\n";

   // return 0;
}
