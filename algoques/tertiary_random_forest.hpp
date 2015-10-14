#pragma once 

#include "utils.hpp"

namespace AlgoComp {

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
  public:
    TertiaryRandomForest ( ): new_output_(false), write_all_output_(false) { }
    
    bool write_all_output_ ;
    //Will be called for initializing the Forest object
    //Can use Forest Class in utils.hpp to parse the forestfile
    void InitializeForest ( const char * const forest_filename_ )
    {
      forest_ = new AlgoComp::Forest(forest_filename_); 
      // Run tests
      // Build Meta Info
      std::vector<AlgoComp::Tree>::iterator it_tree_ = forest_->tree_vec_.begin();
      while(it_tree_ != forest_->tree_vec_.end())
      {
        std::vector<AlgoComp::Node>::iterator it_node_ = (*it_tree_).begin();
        AlgoComp::TreeMetaInfo tree_meta_info_;
        int current_index_ = 0;
        while(it_node_ != (*it_tree_).end())
        {
          AlgoComp::Node current_node_ = *it_node_;
          if ( !current_node_.is_leaf_ )
            tree_meta_info_[current_node_.predictor_index_].push_back( AlgoComp::NodeMetaInfo(current_node_, current_index_));
          it_node_++;
          current_index_++;
        }
        forest_meta_info_.push_back(tree_meta_info_);
        it_tree_++;
      }

      forest_size_ = forest_->tree_vec_.size();
      for(int i=0; i<forest_size_; i++)
        forest_output_.push_back(0.0);
      for(int i=0; i<forest_size_; i++)
        compute_this_tree_.push_back(false);
      tree_threads_ = new std::thread[forest_size_];
    } 

    void PrintForestMetaInfo()
    {
      std::vector<AlgoComp::TreeMetaInfo>::iterator it_tree_;
      it_tree_ = forest_meta_info_.begin();
      int tree_index_ = 0;
      while(it_tree_ != forest_meta_info_.end())
      {
        //std::cout<<"Tree #"<<tree_index_<<std::endl;
        AlgoComp::TreeMetaInfo::iterator it_map_;
        it_map_ = (*it_tree_).begin();
        while(it_map_ != (*it_tree_).end())
        {
          for(int i=0; i<it_map_->second.size(); i++)
            //std::cout<<it_map_->first<<"::"<<it_map_->second[i].predictor_node_loc_<<std::endl;
          ++it_map_;
        }
        ++it_tree_;
        ++tree_index_;
      }
    }

    //Will be called to notfiy changes in predictor values
    //Should return the updated output value
    double OnInputChange ( unsigned int input_index_, double input_value_ )
    { 
      new_output_ = false;
      //std::cout<<input_index_<<"->"<<input_value_<<std::endl;
      std::vector<AlgoComp::TreeMetaInfo>::iterator it_forest_meta_info_;  
      it_forest_meta_info_ = forest_meta_info_.begin();
      predictors_map_[input_index_] = input_value_;
      unsigned int forest_tree_index_ = 0, i, j;
      while(it_forest_meta_info_ != forest_meta_info_.end())
      {
        AlgoComp::TreeMetaInfo tree_meta_info_map_ = *it_forest_meta_info_;
        AlgoComp::TreeMetaInfo::iterator  node_meta_info_ = tree_meta_info_map_.find(input_index_);
        if (node_meta_info_ != tree_meta_info_map_.end())
        {
          //std::cout<<"Inside tree meta #"<<forest_tree_index_<<std::endl;
          std::vector<AlgoComp::NodeMetaInfo>::iterator it_map_value_vec_ = node_meta_info_->second.begin();    
          while(it_map_value_vec_ != node_meta_info_->second.end())
          {
            AlgoComp::NodeMetaInfo node_info_ = *it_map_value_vec_;
            int input_region_;
            if ( input_value_ < node_info_.boundary_value_1_)
              input_region_ = 1;
            else if (input_value_ >= node_info_.boundary_value_1_ && input_value_ < node_info_.boundary_value_2_)
              input_region_ = 2;
            else
              input_region_ = 3;

            if (input_region_ != node_info_.current_region_)
            {
              //std::cout<<"tree index: "<<forest_tree_index_<<std::endl;
              //ComputeNewOutputOnTree( forest_tree_index_, input_index_, input_value_);
              compute_this_tree_[forest_tree_index_] = true;
              new_output_ = true;
              it_map_value_vec_->current_region_ = input_region_;
              // output changed flag
            }
            ++it_map_value_vec_;
          }
        }
        ++forest_tree_index_;
        ++it_forest_meta_info_;
      }
      
      for(j=0; j<forest_size_; j++)
      {
        if( compute_this_tree_[j])
        {
          /*
          thread_create_return_ = pthread_create(&tree_threads_[j], 0, AlgoComp::TertiaryRandomForest::ComputeNewOutputOnTree, (void *)j);  
          if (thread_create_return_ != 0)
            std::cout<<"pthread_create failed"<<std::endl;
          */
          tree_threads_[j] = std::thread(TertiaryRandomForest::ComputeNewOutputOnTree, j);
        }
      }
      for(i=0; i<forest_size_; i++)
      {
        if(compute_this_tree_[i])
        {
          /*
          thread_join_return_ = pthread_join(tree_threads_[i], &thread_return_[i]);  
          if (thread_join_return_ != 0)
            std::cout<<"pthread_join failed"<<std::endl;
          */
          tree_threads_[i].join();
        }
      }

      for(i=0; i<forest_size_; i++)
        compute_this_tree_[i] = false;
      //calculate mean of forest
      double forest_mean_output_ = 0.0;
      //std::cout<<"current predictor index "<<input_index_<<std::endl;
      //std::cout<<new_output_<<" "<<write_all_output_<<std::endl;
      int num_outputs_ = forest_output_.size();
      if(new_output_ || write_all_output_)
      {
        for(i=0; i<num_outputs_; i++)
          forest_mean_output_ += forest_output_[i];

        forest_mean_output_ = forest_mean_output_ / num_outputs_;
        //std::cout<<"forest mean : "<<forest_mean_output_<<std::endl;
        listener_->OnOutputChange(forest_mean_output_);
      }
      //std::cout<<forest_output_[0]<<","<<forest_output_[1]<<std::endl;
      return forest_mean_output_; 
    } 
    
    //_new_listener_ shoud be notified by calling _new_listener_->OnOutputChange on every change in Forest Output
    //should return true if _new_listener_ is successfully subscribed to Forest output updates
    bool SubscribeOutputChange ( OutputChangeListener * _new_listener_ )
    { 
      listener_ = _new_listener_;
      return true;
    } 

    static void ComputeNewOutputOnTree ( unsigned int _index_)
    {
      unsigned int tree_index_ = (unsigned int) _index_;
      AlgoComp::Tree compute_tree_ = forest_->tree_vec_[tree_index_];
      int temp_node_index_ = 0;
      int current_predictor_index_;
      double current_predictor_value_, boundary_value_1_, boundary_value_2_;
      while(!compute_tree_[temp_node_index_].is_leaf_)
      {
        current_predictor_index_ = compute_tree_[temp_node_index_].predictor_index_;
        boundary_value_1_ = compute_tree_[temp_node_index_].boundary_value_vec_[0]; 
        boundary_value_2_ = compute_tree_[temp_node_index_].boundary_value_vec_[1]; 
        std::unordered_map<int, double>::const_iterator current_predictor_it_ = predictors_map_.find(current_predictor_index_); 

        if (current_predictor_it_ != predictors_map_.end())
        {
          current_predictor_value_ = current_predictor_it_->second;
          if (current_predictor_value_ < boundary_value_1_)
            temp_node_index_ = compute_tree_[temp_node_index_].child_node_index_vec_[0];
          else if ( current_predictor_value_ >= boundary_value_1_ && current_predictor_value_ < boundary_value_2_ )
            temp_node_index_ = compute_tree_[temp_node_index_].child_node_index_vec_[1];
          else 
            temp_node_index_ = compute_tree_[temp_node_index_].child_node_index_vec_[2];
          //std::cout<<"jumping to "<<temp_node_index_<<std::endl;
          //std::cout<<current_predictor_index_<<","<<current_predictor_value_<<","<<boundary_value_1_<<","<<boundary_value_2_<<std::endl;
        }
        else
        {
          forest_output_[tree_index_] = 0.0;
          pthread_exit(0);
        }
      }
      forest_output_[tree_index_] = compute_tree_[temp_node_index_].predicted_value_;
      //std::cout<<"tree output: "<<forest_output_[tree_index_]<<std::endl;
      pthread_exit(0);
    }

  private:
    static AlgoComp::Forest *forest_;
    unsigned int forest_size_;
    std::vector<AlgoComp::TreeMetaInfo> forest_meta_info_; 
    static std::vector<double> forest_output_;
    static std::unordered_map<int, double> predictors_map_;
    OutputChangeListener * listener_;
    bool new_output_;
    std::vector<bool> compute_this_tree_;
    std::thread *tree_threads_;
  };

  AlgoComp::Forest * TertiaryRandomForest::forest_;
  std::vector<double> TertiaryRandomForest::forest_output_;
  std::unordered_map<int, double> TertiaryRandomForest::predictors_map_;
  
}
