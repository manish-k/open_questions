#pragma once


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>


namespace AlgoComp {
typedef uint64_t cyclecount_t;

#ifdef _WIN32
//Windows
#include <intrin.h>
static inline cyclecount_t GetCpucycleCount(void)
{
  return __rdtsc();
}

#else
//Linux
static inline cyclecount_t GetCpucycleCount(void)
{
  uint32_t lo, hi;
  __asm__ __volatile__ (      // serialize
      "xorl %%eax,%%eax \n        cpuid"
      ::: "%rax", "%rbx", "%rcx", "%rdx");
  /* We cannot use "=A", since this would use %rax on x86_64 and return only the lower 32bits of the TSC */
  __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
  return (cyclecount_t)hi << 32 | lo;
}

#endif //_WIN32

class PerishableStringTokenizer {
 public:
  std::vector < const char * > tokens_ ;
  PerishableStringTokenizer ( char * t_readline_buffer_, unsigned int _bufferlen_ )
  {
    bool reading_word_ = false;
    for ( unsigned int i = 0 ; i < _bufferlen_ ; i ++ )
    {
      int int_value_char_ = (int)t_readline_buffer_[i] ;

      if ( iscntrl ( int_value_char_ ) && ( t_readline_buffer_[i] != '\t' ) )
      { // end of string .. should be replaced with == NUL ?
        t_readline_buffer_[i] = '\0';
        break;
      }

      if ( ! isspace ( int_value_char_ ) && ! reading_word_ )
      {
        tokens_.push_back ( & ( t_readline_buffer_[i] ) ) ;
        reading_word_ = true;
      }

      if ( isspace ( int_value_char_ ) && reading_word_ )
      { // first space char after word ... replace with NUL
        t_readline_buffer_[i] = '\0';
        reading_word_ = false;
      }
    }
  }
  const std::vector < const char * > & GetTokens ( ) const { return tokens_; }
};

//Sample parsing of forest from a forestfile
class Node
{
 public:
  int node_index_;

  std::vector<int> child_node_index_vec_;     //irrelevant for terminal nodes
  int predictor_index_;                       //irrelevant for terminal nodes
  std::vector<double> boundary_value_vec_;    //irrelevant for terminal nodes

  bool is_leaf_;                              //true for terminal nodes
  double predicted_value_;                    //relevant only for terminal nodes

  Node ( const std::vector<const char *> & treeline_tokens_, int _node_index_ )
      : node_index_(_node_index_)
  {
    if( treeline_tokens_.size() >= 9 )
    {
      child_node_index_vec_.push_back(atoi(treeline_tokens_[1]));
      child_node_index_vec_.push_back(atoi(treeline_tokens_[2]));
      child_node_index_vec_.push_back(atoi(treeline_tokens_[3]));

      predictor_index_ = atoi(treeline_tokens_[4]);

      boundary_value_vec_.push_back(atof(treeline_tokens_[5]));
      boundary_value_vec_.push_back(atof(treeline_tokens_[6]));

      is_leaf_ = strncmp(treeline_tokens_[7],"Y",1)==0;
      predicted_value_ = atof(treeline_tokens_[8]);

    }
  }

};

typedef std::vector<Node> Tree;

class Forest
{
 public:
  std::vector<Tree> tree_vec_;
  int num_predictors_;

  Forest(const char * _forest_filename_ )
      :num_predictors_(0)
  {
    std::ifstream ifs_(_forest_filename_, std::iostream::in);
    if ( ifs_.is_open() )
    {
      const int kbufferlen = 1024;
      char buffer_[kbufferlen];
      while ( ifs_.good() )
      {
        memset( buffer_, '\0' , kbufferlen );
        ifs_.getline( buffer_, kbufferlen );

        PerishableStringTokenizer tokenizer_ ( buffer_, kbufferlen );
        const std::vector< const char * > & tokens_ = tokenizer_.GetTokens();

        if ( tokens_.empty() ) { continue; }

        if ( strcmp(tokens_[0],"NUM_PREDICTORS")==0 && tokens_.size() >= 2 )
        {
          num_predictors_ = atoi(tokens_[1]);
        }
        else if ( strcmp(tokens_[0],"TREESTART")==0 )
        {
          tree_vec_.push_back(Tree());
        }
        else if ( strcmp(tokens_[0],"TREELINE")==0 && tokens_.size() >= 9 )
        {
          if ( ! tree_vec_.empty() )
          {
            Tree & this_tree_ = tree_vec_[tree_vec_.size()-1];
            this_tree_.push_back( Node(tokens_,this_tree_.size()) );
          }
        }
      }
      ifs_.close();
    }
    else
    {
      std::cerr << "ERROR: Cannot open " << _forest_filename_ << " for reading." << std::endl;
      exit ( 0 ) ;
    }

  }

  //some basic sanity checks for validity of forest
  //can add more checks - like sanity of every node
  bool is_valid()
  {
    if ( num_predictors_<=0 || tree_vec_.empty() ) { return false; }

    for ( unsigned int i=0; i<tree_vec_.size(); i++ )
    {
      if ( tree_vec_[i].empty() ) { return false; }
    }

    return true;
  }
};

}

