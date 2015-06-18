The challenge here is to write a very fast and correct implementation of a tertiary random forest.

http://www.circulumvite.com/home/competitions-and-open-questions/algo-competition

Motivation :
A majority of data science applications also emphasize speed of computation along with accuracy. There is anecdotal evidence of the fact that non linear decision tree methods have performed well in a variety of data science applications. The challenge of using it in the domain of HFT will be, among others, implementing it efficiently. On most events, there are some "knee-jerk" reactions where the response function is largely simple and computing the short term effect of the event is easy. Then there are more complex responses. To model them we need more sophisticated perhaps non-linear models that take more time. Using a sophisticated model for an elementary prediction would leave it vulnerable to be too slow at the task. This problem is very prevalent in the domain of finance. In finance, one encounters relationships that hold in even very small durations, like in ten seconds after an event has occurred. Then there are relationships that seem to not hold consistently over small durations but show up more often when one looks at longer periods like months and years.

Input:
A data set of indicator values which have been snapped at regular intervals. We have written a wrapper file that reads the data and calls the function OnInputChange on the TertiaryRandomForest class. The two arguments of the function are the index of the input variable that has changed, and the new value. For instance if indicator 5 has changed and the new value of the indicator is -1, process_data.cpp will call OnInputChange ( 5, -1 ) on the TertiaryRandomForest. Note that indicator indices start with 1. Also the starting values of the indicators can be assumed to be 0. This is just relevant for initialization. 

Sample inputdatafiles are available at https://s3.amazonaws.com/dvcpublic/testalgodata.zip

Output:
To measure correctness, at every 'samplingrate' number of function calls, the predicted value is printed. We will try to compare our benchmark solution to yours, and as long as every prediction differs by not more than 1%, we will consider the values to be correct. We allow a margin of error to account for any floating point errors in computation as well as allow any optimizations that might be possible with approximate computations. In this domain a very small difference in predicted price should not affect the outcome. If that margin of error allows one to reduce latency the benefit is often more than the cost.

Deliverables:
1) Code : Submit an implementation of TertiaryRandomForest class in tertiary_random_forest.hpp . process_data.cpp demonstrates one of the ways in which TertiaryRandomForest can be used, submitted implementation should be generic so that one can use it as an API. Use of SubscribeOutputChange is not demonstrated in process_data.cpp but it should be implemented so that subcribers should recieve all the output changes from TertiaryRandomForest. 
2) Documentation : It should cover -
    i)    overview of logic used to implement the forest  
    ii)   any critical design decisions 
    iii)  any testing to check the correctness of logic.

Files:
1) process_data.cpp : This is the file with the main function. To build the exec please run "g++ -std=gnu++0x -O3 process_data.cpp -o algoques_exec".
2) tertiary_random_forest.hpp : This is the file you need to complete the implementations in!
3) RANDOM_FOREST_DESCRIPTION.txt : This describes the structure of the forest info file.
4) utils.hpp : helper utilities
5) sample_random_forest.txt : This is a sample file describing a random tertiary forest compliant with the specifications in RANDOM_FOREST_DESCRIPTION.
6) prod_data_20131213v.txt : This is a sample "inputdatafile", that is the second argument of algoques_exec.
7) Sample_Random_Forest_Spec.pdf : A graphical representation of a random forest file.

Build :
g++ -std=gnu++0x -O3 process_data.cpp -o algoques_exec
or
g++ -O3 process_data.cpp -o algoques_exec

Run:
If the exec name is algoques_exec, then a sample command would be :
./algoques_exec forestfile inputdatafile outputdatafile 
It prints two outputs :
  - the name of the output file where it wrote the sampled target values.
  - the total CPUcycles taken in the operation.
