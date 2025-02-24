#C++ skeleton for Bison

#Copyright(C) 2002 - 2015, 2018 - 2021 Free Software Foundation, Inc.

#This program is free software : you can redistribute it and / or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.If not, see < https:  // www.gnu.org/licenses/>.

m4_pushdef([b4_copyright_years],
           [2002-2015, 2018-2021])

#b4_location_file
#-- -- -- -- -- -- -- --
#Name of the file containing the position / location class,
#if we want this file.
b4_percent_define_check_file([b4_location_file],
                             [[api.location.file]],
                             b4_header_if([[location.hh]]))

#    b4_location_include
#    -- -- -- -- -- -- -- -- -- -
#    If location.hh is to be generated, the name under which should it be
#    included.
#
#    b4_location_path
#    -- -- -- -- -- -- -- --
#    The path to use for the CPP guard.
m4_ifdef([b4_location_file],
[m4_define([b4_location_include],
           [b4_percent_define_get([[api.location.include]],
                                  ["b4_location_file"])])
 m4_define([b4_location_path],
           b4_percent_define_get([[api.location.include]],
                                 ["b4_mapped_dir_prefix[]b4_location_file"]))
 m4_define([b4_location_path],
           m4_substr(m4_defn([b4_location_path]), 1, m4_eval(m4_len(m4_defn([b4_location_path])) - 2)))
 ])

#    b4_position_file
#    -- -- -- -- -- -- -- --
#    Name of the file containing the position class, if we want this file.
b4_header_if(
  [b4_required_version_if(
    [30200], [],
    [m4_ifdef([b4_location_file],
              [m4_define([b4_position_file], [position.hh])])])])

#    b4_location_define
#    -- -- -- -- -- -- -- -- --
#    Define the position and location classes.
m4_define([b4_location_define],
[[  /// A point in a source file.
  class position
  {
  public:
    /// Type for file name.
    typedef ]b4_percent_define_get([[api.filename.type]])[ filename_type;
    /// Type for line and column numbers.
    typedef int counter_type;
]m4_ifdef([b4_location_constructors], [[
    /// Construct a position.
    explicit position (filename_type* f = YY_NULLPTR,
                       counter_type l = ]b4_location_initial_line[,
                       counter_type c = ]b4_location_initial_column[)
      : filename (f)
      , line (l)
      , column (c)
    {}

]])[
    /// Initialization.
    void initialize (filename_type* fn = YY_NULLPTR,
                     counter_type l = ]b4_location_initial_line[,
                     counter_type c = ]b4_location_initial_column[)
    {
        filename = fn;
        line     = l;
        column   = c;
    }

    /** \name Line and Column related manipulators
     ** \{ */
    /// (line related) Advance to the COUNT next lines.
    void lines (counter_type count = 1)
    {
        if (count)
        {
          column = ]b4_location_initial_column[;
          line = add_ (line, count, ]b4_location_initial_line[);
        }
    }

    /// (column related) Advance to the COUNT next columns.
    void columns (counter_type count = 1)
    {
      column = add_ (column, count, ]b4_location_initial_column[);
    }
    /** \} */

    /// File name to which this position refers.
    filename_type* filename;
    /// Current line number.
    counter_type line;
    /// Current column number.
    counter_type column;

  private:
    /// Compute max (min, lhs+rhs).
    static counter_type add_ (counter_type lhs, counter_type rhs, counter_type min)
    {
        return lhs + rhs < min ? min : lhs + rhs;
    }
  };

  /// Add \a width columns, in place.
  inline position&
  operator+= (position& res, position::counter_type width)
  {
    res.columns(width);
    return res;
  }

  /// Add \a width columns.
  inline position
  operator+ (position res, position::counter_type width)
  {
    return res += width;
  }

  /// Subtract \a width columns, in place.
  inline position&
  operator-= (position& res, position::counter_type width)
  {
    return res += -width;
  }

  /// Subtract \a width columns.
  inline position
  operator- (position res, position::counter_type width)
  {
    return res -= width;
  }
]b4_percent_define_flag_if([[define_location_comparison]], [[
  /// Compare two position objects.
  inline bool
  operator== (const position& pos1, const position& pos2)
  {
    return (pos1.line == pos2.line && pos1.column == pos2.column &&
            (pos1.filename == pos2.filename ||
             (pos1.filename && pos2.filename && *pos1.filename == *pos2.filename)));
  }

  /// Compare two position objects.
  inline bool
  operator!= (const position& pos1, const position& pos2)
  {
    return !(pos1 == pos2);
  }
]])[
  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param pos a reference to the position to redirect
   */
  template <typename YYChar>
  std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const position& pos)
  {
    if (pos.filename)
        ostr << *pos.filename << ':';
    return ostr << pos.line << '.' << pos.column;
  }

  /// Two points in a source file.
  class location
  {
  public:
    /// Type for file name.
    typedef position::filename_type filename_type;
    /// Type for line and column numbers.
    typedef position::counter_type counter_type;
]m4_ifdef([b4_location_constructors], [
    /// Construct a location from \a b to \a e.
    location (const position& b, const position& e)
      : begin (b)
      , end (e)
    {}

    /// Construct a 0-width location in \a p.
    explicit location (const position& p = position ())
      : begin (p)
      , end (p)
    {}

    /// Construct a 0-width location in \a f, \a l, \a c.
    explicit location (filename_type* f,
                       counter_type l = ]b4_location_initial_line[,
                       counter_type c = ]b4_location_initial_column[)
      : begin (f, l, c)
      , end (f, l, c)
    {}

])[
    /// Initialization.
    void initialize (filename_type* f = YY_NULLPTR,
                     counter_type l = ]b4_location_initial_line[,
                     counter_type c = ]b4_location_initial_column[)
    {
        begin.initialize(f, l, c);
        end = begin;
    }

    /** \name Line and Column related manipulators
     ** \{ */
  public:
    /// Reset initial location to final location.
    void step ()
    {
        begin = end;
    }

    /// Extend the current location to the COUNT next columns.
    void columns (counter_type count = 1)
    {
        end += count;
    }

    /// Extend the current location to the COUNT next lines.
    void lines (counter_type count = 1)
    {
        end.lines(count);
    }
    /** \} */


  public:
    /// Beginning of the located region.
    position begin;
    /// End of the located region.
    position end;
  };

  /// Join two locations, in place.
  inline location&
  operator+= (location& res, const location& end)
  {
    res.end = end.end;
    return res;
  }

  /// Join two locations.
  inline location
  operator+ (location res, const location& end)
  {
    return res += end;
  }

  /// Add \a width columns to the end position, in place.
  inline location&
  operator+= (location& res, location::counter_type width)
  {
    res.columns(width);
    return res;
  }

  /// Add \a width columns to the end position.
  inline location
  operator+ (location res, location::counter_type width)
  {
    return res += width;
  }

  /// Subtract \a width columns to the end position, in place.
  inline location&
  operator-= (location& res, location::counter_type width)
  {
    return res += -width;
  }

  /// Subtract \a width columns to the end position.
  inline location
  operator- (location res, location::counter_type width)
  {
    return res -= width;
  }
]b4_percent_define_flag_if([[define_location_comparison]], [[
  /// Compare two location objects.
  inline bool
  operator== (const location& loc1, const location& loc2)
  {
    return loc1.begin == loc2.begin && loc1.end == loc2.end;
  }

  /// Compare two location objects.
  inline bool
  operator!= (const location& loc1, const location& loc2)
  {
    return !(loc1 == loc2);
  }
]])[
  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param loc a reference to the location to redirect
   **
   ** Avoid duplicate information.
   */
  template <typename YYChar>
  std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const location& loc)
  {
    location::counter_type end_col = 0 < loc.end.column ? loc.end.column - 1 : 0;
    ostr << loc.begin;
    if (loc.end.filename && (!loc.begin.filename || *loc.begin.filename != *loc.end.filename))
        ostr << '-' << loc.end.filename << ':' << loc.end.line << '.' << end_col;
    else if (loc.begin.line < loc.end.line)
        ostr << '-' << loc.end.line << '.' << end_col;
    else if (loc.begin.column < end_col)
        ostr << '-' << end_col;
    return ostr;
  }
]])


m4_ifdef([b4_position_file], [[
]b4_output_begin([b4_dir_prefix], [b4_position_file])[
]b4_generated_by[
// Starting with Bison 3.2, this file is useless: the structure it
// used to define is now defined in "]b4_location_file[".
//
// To get rid of this file:
// 1. add '%require "3.2"' (or newer) to your grammar file
// 2. remove references to this file from your build system
// 3. if you used to include it, include "]b4_location_file[" instead.

#    include] b4_location_include[
]b4_output_end[
]])


m4_ifdef([b4_location_file], [[
]b4_output_begin([b4_dir_prefix], [b4_location_file])[
]b4_copyright([Locations for Bison parsers in C++])[
/**
 ** \file ]b4_location_path[
 ** Define the ]b4_namespace_ref[::location class.
 */

]b4_cpp_guard_open([b4_location_path])[

#    include <iostream>
#    include <string>

]b4_null_define[

]b4_namespace_open[
]b4_location_define[
]b4_namespace_close[
]b4_cpp_guard_close([b4_location_path])[
]b4_output_end[
]])


m4_popdef([b4_copyright_years])
