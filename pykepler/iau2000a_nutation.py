# iau2000a_nutation.py - Wrapper for IAU2000A nutation routines
# Copyright (C) 2010 Shiva Iyer <shiva.iyer AT g m a i l DOT c o m>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if __name__ == "__main__":
    exit()

from ctypes import *
from julian_date import *
from pykepler import _libkepler

def iau2000a_mean_obliquity(tdb):

    return _libkepler.iau2000a_mean_obliquity(byref(tdb))

def iau2000a_nutation(tdb):

    nut_longitude = c_double()
    nut_latitude = c_double()

    _libkepler.iau2000a_nutation(byref(tdb), byref(nut_longitude),
                                 byref(nut_latitude))

    return nut_longitude.value, nut_latitude.value

def iau2000a_nutation_matrix(tdb):

    nut_matrix = (c_double * 9)()

    _libkepler.iau2000a_nutation_matrix(byref(tdb), pointer(nut_matrix))

    return [nut_matrix[0:3], nut_matrix[3:6], nut_matrix[6:9]]

_libkepler.iau2000a_mean_obliquity.restype = c_double
_libkepler.iau2000a_mean_obliquity.argtypes = [
    POINTER(JulianDate)
]

_libkepler.iau2000a_nutation.restype = None
_libkepler.iau2000a_nutation.argtypes = [
    POINTER(JulianDate),
    POINTER(c_double),
    POINTER(c_double)
]

_libkepler.iau2000a_nutation_matrix.restype = None
_libkepler.iau2000a_nutation_matrix.argtypes = [
    POINTER(JulianDate),
    POINTER(c_double * 9)
]

__all__ = [
    "iau2000a_mean_obliquity",
    "iau2000a_nutation",
    "iau2000a_nutation_matrix"
]
