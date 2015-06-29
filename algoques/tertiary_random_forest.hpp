#pragma once 

#include "utils.hpp"
#include <sstream>

namespace AlgoComp {

struct TreeLine {
  int first_child_;
  int second_child_;
  int third_child_;
  int split_indicator_index_ ;
  double split_indicator_value1_ ;
  double split_indicator_value2_ ;

  bool is_leaf_;

  double prediction_;
  int node_answer_;

  TreeLine ( )
      : first_child_ ( -1 ),
      second_child_ ( -1 ),
      third_child_ ( -1 ),
      split_indicator_index_ (-1 ),
      split_indicator_value1_ ( 0.0 ),
      split_indicator_value2_ ( 0.0 ),
      is_leaf_ ( false ),
      prediction_ ( 0.0 ),
      node_answer_ ( -1 )
  {
  }

  TreeLine ( const TreeLine & _new_tree_line_)
      : first_child_ ( _new_tree_line_.first_child_ ),
      second_child_ ( _new_tree_line_.second_child_ ),
      third_child_ ( _new_tree_line_.third_child_ ),
      split_indicator_index_ ( _new_tree_line_.split_indicator_index_ ),
      split_indicator_value1_ ( _new_tree_line_.split_indicator_value1_ ),
      split_indicator_value2_ ( _new_tree_line_.split_indicator_value2_ ),
      is_leaf_ ( _new_tree_line_.is_leaf_ ),
      prediction_ ( _new_tree_line_.prediction_ ),
      node_answer_ ( _new_tree_line_.node_answer_ )
  {
  }

  inline std::string toString () const
  {
    std::ostringstream t_temp_oss_ ;

    t_temp_oss_ << "TreeLine" << ' '
        << first_child_ << ' '
        << second_child_ << ' '
        << third_child_ << ' ' 
        << split_indicator_index_ << ' '
        << split_indicator_value1_ << ' '
        << split_indicator_value2_ << ' '
        << is_leaf_ << ' '
        << prediction_ << ' '
        << node_answer_ << ' '
        << "\n";

    return t_temp_oss_.str();
  }
};

class InputChangeListener {
 public:
  virtual ~InputChangeListener ( ) { };

  virtual double OnInputChange ( unsigned int input_index_, double input_value_ ) = 0;
};

class OutputChangeListener {
 public:
  virtual ~OutputChangeListener ( ) { };

  virtual void OnOutputChange ( double _new_out_value_ ) = 0;
};


class TertiaryRandomForest : public InputChangeListener {
 private:

  std::vector < double > prev_value_vec_ ;

  double sum_vars_ ;
  double last_propagated_target_price_ ;

  std::vector < std::vector < TreeLine * > > indicator_based_nodes_vec_;
  std::vector < std::vector <  int > > indicator_based_tree_vec_;

  unsigned int num_trees_ ;
  double normalization_factor_;

  std::vector < std::vector < TreeLine * > > tree_vec_ ;
  std::vector < double > tree_predictions_ ;
  std::vector < OutputChangeListener * > output_listeners_vec_;

 public:
  TertiaryRandomForest ( )
      : prev_value_vec_(), sum_vars_ ( 0 ), last_propagated_target_price_ ( 0 ),
      indicator_based_nodes_vec_ (), indicator_based_tree_vec_ (),
      num_trees_ ( 0 ),
      normalization_factor_ ( 0.0 ),
      tree_vec_ (),
      tree_predictions_ (),
      output_listeners_vec_()
  {
    prev_value_vec_.clear();
  }

  ~TertiaryRandomForest ( ) { }
  
  bool SubscribeOutputChange ( OutputChangeListener * _new_listener_ ) 
  { 
    if ( _new_listener_ != NULL ) 
    {
      output_listeners_vec_.push_back(_new_listener_);
      return true;
    }
    return false;
  }

  void NotifyListeners ( )
  {
    for ( unsigned int i = 0; i<output_listeners_vec_.size(); i++ )
    {
      output_listeners_vec_[i]->OnOutputChange(sum_vars_/num_trees_);
    }
  }  



  void InitializeForest ( const char * const forest_filename_ ) 
  { 
    std::ifstream model_infile_;
    model_infile_.open ( forest_filename_, std::ifstream::in ) ;
    if ( model_infile_.is_open ( ) )
    {
      const unsigned int kModelLineBufferLen = 1024 ;
      char readline_buffer_ [ kModelLineBufferLen ];
      bzero ( readline_buffer_, kModelLineBufferLen );
      unsigned int tree_count_ = 0;
      unsigned int tree_line_count_ = 0;

      while ( model_infile_.good ( ) )
      {
        bzero ( readline_buffer_, kModelLineBufferLen );
        model_infile_.getline ( readline_buffer_, kModelLineBufferLen ) ;
        if ( model_infile_.gcount ( ) > 0 )
        {
          PerishableStringTokenizer st_ ( readline_buffer_, kModelLineBufferLen ); // Perishable string readline_buffer_
          const std::vector < const char * > & tokens_ = st_.GetTokens ( );

          if ( tokens_.size ( ) < 1 )
          { // skip empty lines
            continue;
          }

          if ( strcmp ( tokens_[0], "NUM_PREDICTORS" ) == 0 )
          {
            int num_indicators_ = atoi ( tokens_[1] ); 			 	
            for ( unsigned int i=0; i < num_indicators_ ; i++ )
            {			 
              prev_value_vec_.push_back ( 0.0 ) ;
              std::vector < unsigned int > t_indicator_based_trees_ ;
              std::vector < TreeLine * > indicator_based_nodes_;
              std::vector < int > indicator_based_trees_;
              indicator_based_trees_.push_back ( -1 );	
              TreeLine * t_line_ = new TreeLine ();
              indicator_based_nodes_.push_back ( t_line_ );
              indicator_based_nodes_vec_.push_back ( indicator_based_nodes_ );
              indicator_based_tree_vec_.push_back ( indicator_based_trees_ );
            }
          }

          if ( strcmp ( tokens_[0], "TREESTART" ) == 0 )
          {
            AddTree( );
            tree_count_ ++ ;
            tree_line_count_ = 0;
          }
          else if ( strcmp ( tokens_[0], "TREELINE" ) == 0 )
          {
            unsigned int first_child_ = atoi ( tokens_[1] );
            unsigned int second_child_ = atoi ( tokens_[2] );
            unsigned int third_child_ = atoi ( tokens_[3] );
            unsigned int split_indicator_index_ = atoi ( tokens_[4] );	    
            double split_indicator_value1_ = atof ( tokens_[5] );
            double split_indicator_value2_ = atof ( tokens_[6] );
            bool is_leaf_ = true;
            if ( strcmp ( tokens_[7], "N" ) == 0 )
            {
              is_leaf_ = false;
            }
            double prediction_ = atof ( tokens_[8] );
            AddTreeLine( tree_count_-1, tree_line_count_, split_indicator_index_, first_child_, second_child_, third_child_, split_indicator_value1_, split_indicator_value2_, prediction_, is_leaf_ );
            tree_line_count_ ++ ;
          }
        }
      }	
      num_trees_ = tree_count_ ;
    } 	 
    else
    {
      std::cerr << "ERROR: Cannot open " << forest_filename_ << " for reading." << std::endl;
      exit ( 0 ) ;
    }
  }

  void AddTreeLine ( const unsigned int tree_index_, const unsigned int tree_line_index_, const int t_indicator_index_ , const int t_first_child_, const int t_second_child_, const int t_third_child_, const double t_split_value1_, const double t_split_value2_, const double t_prediction_, const bool t_is_leaf_ )
  {
    TreeLine * t_line_ = new TreeLine ();
    t_line_->first_child_ = t_first_child_;
    t_line_->second_child_ = t_second_child_;
    t_line_->third_child_ = t_third_child_;
    t_line_->split_indicator_index_ = t_indicator_index_;
    t_line_->split_indicator_value1_ = t_split_value1_;
    t_line_->split_indicator_value2_ = t_split_value2_;
    if ( t_is_leaf_ )
    {
      t_line_->prediction_ = t_prediction_;
    }
    else
    {
      t_line_->prediction_ = 0.0;
    }
    t_line_->is_leaf_ = t_is_leaf_;
    t_line_->node_answer_ = -1;

    if ( tree_line_index_ <= 0 )
    {
      std::vector < TreeLine * > t_tree_;
      t_tree_.push_back ( t_line_ );
      tree_vec_.push_back ( t_tree_ );
    }
    else
    {
      tree_vec_[tree_index_].push_back (t_line_);
    }

    if ( t_indicator_index_ >= 0 )
    {
      indicator_based_nodes_vec_[t_indicator_index_].push_back ( t_line_ );
      indicator_based_tree_vec_[t_indicator_index_].push_back ( tree_index_ );
    }
  }

  double OnInputChange ( unsigned int input_index_, double input_value_ ) 
  { 	  
    if ( indicator_based_tree_vec_ [ input_index_ ].size() <= 1 )
    {
      NotifyListeners();
      return sum_vars_ / num_trees_;
    }
    for ( unsigned int i = 0; i < indicator_based_nodes_vec_[input_index_].size(); i++ )
    {
      if ( input_value_ <= indicator_based_nodes_vec_ [ input_index_ ][i]->split_indicator_value1_ )
      {
        indicator_based_nodes_vec_ [ input_index_ ][i]->node_answer_ = 1;
      }
      else if ( input_value_ > indicator_based_nodes_vec_ [ input_index_ ][i]->split_indicator_value2_ )
      {
        indicator_based_nodes_vec_ [ input_index_ ][i]->node_answer_ = 3;
      }
      else
      {
        indicator_based_nodes_vec_ [ input_index_ ][i]->node_answer_ = 2;
      }
    }

    for ( unsigned int i = 1; i < indicator_based_tree_vec_[input_index_].size(); i++ )
    {
      int current_index_ = 0;
      while ( ! tree_vec_[indicator_based_tree_vec_[ input_index_ ][i]][current_index_]->is_leaf_ )
      {
        if ( tree_vec_[indicator_based_tree_vec_[ input_index_ ][i]][current_index_]->node_answer_ == 1 )
        {
          current_index_ = tree_vec_[indicator_based_tree_vec_[ input_index_ ][i]][current_index_]->first_child_;
        }
        else if ( tree_vec_[indicator_based_tree_vec_ [ input_index_ ][i]][current_index_]->node_answer_ == 2 )
        {
          current_index_ = tree_vec_[indicator_based_tree_vec_[ input_index_ ][i]][current_index_]->second_child_;
        }
        else if ( tree_vec_[indicator_based_tree_vec_ [ input_index_ ][i]][current_index_]->node_answer_ == 3 )
        {
          current_index_ = tree_vec_[indicator_based_tree_vec_[ input_index_ ][i]][current_index_]->third_child_;
        }
        else
        {
          break;
        }
      }

      sum_vars_ = sum_vars_ - tree_predictions_[indicator_based_tree_vec_[input_index_][i]];
      tree_predictions_[indicator_based_tree_vec_[input_index_][i]] = tree_vec_[indicator_based_tree_vec_[ input_index_ ][i]][current_index_]->prediction_;
      sum_vars_ = sum_vars_ + tree_predictions_[indicator_based_tree_vec_[input_index_][i]];
    }
    prev_value_vec_ [ input_index_ ] = input_value_ ;	  
    NotifyListeners();
    return sum_vars_ / num_trees_;
  } 

  void AddTree ( )
  {
    tree_predictions_.push_back ( 0 );
  }

};

}
