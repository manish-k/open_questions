#!/usr/bin/env python
import os
import sys
import math
import numpy
import glob
import pandas as pd
import itertools
import smtplib

import csv
from numpy import array

from solution import get_weights
from prettytable import PrettyTable

TESTS_FOLDER = os.path.expanduser( "~/open_questions/online_learning_challenge/tests/" )

#Function to calculate daily log returns using actual returns and weights assigned to each signal
#Returns array of daily log returns
def compute_log_returns(_logret_matrix, _weight_matrix):
    _log_returns = numpy.empty(shape=(0))
    for i in xrange(0, _logret_matrix.shape[0]):
        _log_return = math.log(numpy.sum(_weight_matrix[i] * numpy.exp((_logret_matrix[i]))))
        #Append to the series of returns
        _log_returns = numpy.append(_log_returns, _log_return)
    return _log_returns

def perfstats(_log_returns):
    _annualized_percent_returns = (math.exp(252*numpy.mean(_log_returns))-1)*100
    _estimate_of_annual_range = math.sqrt(252.0) * numpy.std(_log_returns)
    _annualized_percent_stdev = ((math.exp(_estimate_of_annual_range) - 1) + (1 - math.exp(-_estimate_of_annual_range)))/2.0 * 100.0
    _sharpe = _annualized_percent_returns/_annualized_percent_stdev
    return _annualized_percent_returns, _annualized_percent_stdev, _sharpe

#Function to compute annualised percent returns using log returns and weights assigned to the signals
def getPerformanceStats(_logret_matrix, _weight_matrix):
    _log_returns = compute_log_returns(_logret_matrix, _weight_matrix)
    _net_log_returns = numpy.sum(_log_returns)
    _annualized_percent_returns, _annualized_percent_stdev, _sharpe = perfstats(_log_returns)
    return ( _annualized_percent_returns, _annualized_percent_stdev, _sharpe )

def run( ):
    perf_stats = PrettyTable(["TestCase", "Annualized Returns", "Annualized Volatility", "Sharpe"])
    for i, filename in enumerate( glob.glob( TESTS_FOLDER + '/*csv' ) ):
        ret_frame = pd.DataFrame.from_csv( filename )
        logret_matrix = ret_frame.values
        weight_matrix = numpy.empty( logret_matrix.shape )
        weight_matrix[ 0, : ] = numpy.array( [ 1.0 / logret_matrix.shape[ 1 ] ] * logret_matrix.shape[ 1 ] )
        for day in range( logret_matrix.shape[ 0 ] -1 ):
            weight_matrix[ day + 1, : ] = get_weights( logret_matrix[ : day, : ] )
        perf_stats.add_row( [ i + 1 ] + list( getPerformanceStats( logret_matrix, weight_matrix ) ) )
    perf_stats.align = "c"
    print perf_stats
    return perf_stats

def send_mail( subject, body, tos ):
    server = "localhost"
    sender = "sanchit.gupta@tworoads.co.in"
    to = ';'.join( tos )
    # Prepare actual message
    message = "From: {0}\nTo: {1}\nSubject: {2}\n\n{3}".format( sender, to, subject, body )
    # Send the mail
    server = smtplib.SMTP( server )
    server.sendmail( sender, to, message )
    server.quit( )

def main( ):
    results = run( )
    body = results.get_string( )
    send_mail( 'Results', body, [ sys.argv[ 1 ], 'sanchit@qplum.co' ] )

if __name__ == '__main__':
    main( )
