#!/usr/bin/env python

import sys
import math
import numpy

def get_weights( logret_matrix ):
    return numpy.array( [ 1.0 / logret_matrix.shape[ 1 ] ] * logret_matrix.shape[ 1 ] )

