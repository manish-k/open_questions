#!/usr/bin/env Rscript
#suppressPackageStartupMessages( library ( "numDeriv" , lib.loc="/apps/R/root/library/" ) );
.libPaths("/apps/R/root/library/")

args = commandArgs( trailingOnly=TRUE )
if ( length(args) < 2 ) {
        stop ("USAGE : <script> <correct_output_file> <test_output_file> \n");
}

correct_output <- as.matrix( read.table( args[1] ) );
test_output <- as.matrix ( read.table ( args[2] ) );


abs_mean <- mean(abs(correct_output[,1]));
abs_diff <- abs(correct_output[,1] - test_output[,1] );

stdev <- sd ( correct_output[,1] );

cat ( "Mean abs error / Mean abs value " , mean(abs_diff)/abs_mean, "\n");
cat ( "Mean abs error / Stdev ", mean(abs_diff)/stdev, "\n" );
cat ( "Max abs error / Mean abs value " , max(abs_diff)/abs_mean,"\n" );
cat ( "Max abs error / Stdev ", max(abs_diff)/stdev,"\n" );
