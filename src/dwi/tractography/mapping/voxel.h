/*
    Copyright 2011 Brain Research Institute, Melbourne, Australia

    Written by Robert E. Smith, 2011.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __dwi_tractography_mapping_voxel_h__
#define __dwi_tractography_mapping_voxel_h__



#include <set>

#include "point.h"

#include "image/info.h"


namespace MR {
namespace DWI {
namespace Tractography {
namespace Mapping {



class Voxel : public Point<int>
{
  public:
    Voxel (const int x, const int y, const int z) : length (1.0f) { p[0] = x; p[1] = y; p[2] = z; }
    Voxel (const Point<int>& that) : Point<int> (that), length (1.0f) { }
    Voxel (const Point<int>& v, const float l) : Point<int> (v), length (l) { }
    Voxel () : length (0.0f) { memset (p, 0x00, 3 * sizeof(int)); }
    bool operator< (const Voxel& V) const { return ((p[2] == V.p[2]) ? ((p[1] == V.p[1]) ? (p[0] < V.p[0]) : (p[1] < V.p[1])) : (p[2] < V.p[2])); }
    Voxel& operator= (const Voxel& V) { Point<int>::operator= (V); length = V.length; return *this; }
    void operator+= (const float l) const { length += l; }
    void normalise() const { length = 1.0f; }
    float get_length() const { return length; }
  private:
    mutable float length;
};






inline Voxel round (const Point<float>& p)
{ 
  assert (std::isfinite (p[0]) && std::isfinite (p[1]) && std::isfinite (p[2]));
  return (Voxel (Math::round<int> (p[0]), Math::round<int> (p[1]), Math::round<int> (p[2])));
}

inline bool check (const Voxel& V, const Image::Info& H)
{
  return (V[0] >= 0 && V[0] < H.dim(0) && V[1] >= 0 && V[1] < H.dim(1) && V[2] >= 0 && V[2] < H.dim(2));
}

inline Point<float> vec2DEC (const Point<float>& d)
{
  return (Point<float> (Math::abs(d[0]), Math::abs(d[1]), Math::abs(d[2])));
}





class VoxelDEC : public Voxel 
{

  public:
    VoxelDEC () :
        Voxel (),
        colour (Point<float> (0.0f, 0.0f, 0.0f)) { }

    VoxelDEC (const Voxel& V) :
        Voxel (V),
        colour (Point<float> (0.0f, 0.0f, 0.0f)) { }

    VoxelDEC (const Voxel& V, const Point<float>& d) :
        Voxel (V),
        colour (vec2DEC (d)) { }

    VoxelDEC (const Voxel& V, const Point<float>& d, const float l) :
        Voxel (V, l),
        colour (vec2DEC (d)) { }

    VoxelDEC& operator=  (const VoxelDEC& V)       { Voxel::operator= (V); colour = V.colour; return (*this); }
    VoxelDEC& operator=  (const Voxel& V)          { Voxel::operator= (V); colour = Point<float> (0.0f, 0.0f, 0.0f); return (*this); }

    // For sorting / inserting, want to identify the same voxel, even if the colour is different
    bool      operator== (const VoxelDEC& V) const { return Voxel::operator== (V); }
    bool      operator<  (const VoxelDEC& V) const { return Voxel::operator< (V); }

    void normalise() const { Voxel::normalise(); colour.normalise(); }
    void set_dir (const Point<float>& i) { colour =  vec2DEC (i); }
    void add (const Point<float>& i, const float l) const { Voxel::operator+= (l); colour += vec2DEC (i); }
    void operator+= (const Point<float>& i) const { Voxel::operator+= (1.0f); colour += vec2DEC (i); }
    const Point<float>& get_colour() const { return colour; }

  private:
    mutable Point<float> colour;

};



// TODO Better handling of Gaussian smoothing case, where a factor is required per voxel for every streamline
// Perhaps even branch the MapWriter: Have the base classes jsut load the factor from SetVoxelExtras, and a
//   derived Gaussian version that loads from the set
/*
class VoxelFactor : public Voxel
{

  public:
    VoxelFactor () :
        Voxel (),
        sum (0.0f),
        contributions (0) { }

    VoxelFactor (const int x, const int y, const int z, const float factor) :
        Voxel (x, y, z),
        sum (factor),
        contributions (1) { }

    VoxelFactor (const Voxel& v) :
        Voxel (v),
        sum (0.0f),
        contributions (0) { }

    template <class Init>
    VoxelFactor (const Init& v, const float f) :
        Voxel (v),
        sum (f),
        contributions (1) { }

    VoxelFactor (const VoxelFactor& v) :
        Voxel (v),
        sum (v.sum),
        contributions (v.contributions) { }

    void add_contribution (const float factor) const {
        sum += factor;
        ++contributions;
    }

    void set_factor (const float i) const { sum = i; contributions = 1; }
    float get_factor() const { return (sum / float(contributions)); }
    size_t get_contribution_count() const { return contributions; }

    VoxelFactor& operator=  (const Voxel& V)             { Voxel::operator= (V); sum = 0.0f; contributions = 0; return (*this); }
    VoxelFactor& operator=  (const VoxelFactor& V)       { Voxel::operator= (V); sum = V.sum; contributions = V.contributions; return (*this); }
    bool         operator== (const VoxelFactor& V) const { return Voxel::operator== (V); }
    bool         operator<  (const VoxelFactor& V) const { return Voxel::operator< (V); }


  protected:
    mutable float sum;
    mutable size_t contributions;

};


class VoxelDECFactor : public VoxelFactor
{

  public:
    VoxelDECFactor () :
        VoxelFactor (),
        colour (Point<float> (0.0f, 0.0f, 0.0f)) { }

    VoxelDECFactor (const Voxel& V) :
        VoxelFactor (V),
        colour (Point<float> (0.0, 0.0, 0.0)) { }

    VoxelDECFactor (const VoxelDECFactor& that) :
        VoxelFactor (that),
        colour (that.colour) { }

    VoxelDECFactor& operator=  (const Voxel& V)                { VoxelFactor::operator= (V); colour = Point<float> (0.0f, 0.0f, 0.0f); return (*this); }
    VoxelDECFactor& operator=  (const VoxelDECFactor& V)       { VoxelFactor::operator= (V); colour = that.colour; return (*this); }
    bool            operator== (const Voxel& V)          const { return Voxel::operator== (V); }
    bool            operator<  (const VoxelDECFactor& V) const { return Voxel::operator< (V); }

    void normalise() const { colour.normalise(); }
    void set_dir (const Point<float>& i)       { colour =  vec2DEC (i); }
    void add_dir (const Point<float>& i) const { colour += vec2DEC (i); }
    const Point<float>& get_colour() const { return colour; }

  private:
    mutable Point<float> colour;

};
*/



// Assumes tangent has been mapped to a hemisphere basis direction set
class Dixel : public Voxel
{

  public:
    Dixel () :
        Voxel (),
        dir (invalid) { }

    Dixel (const Voxel& V) :
        Voxel (V),
        dir (invalid) { }

    Dixel (const Voxel& V, const size_t b) :
        Voxel (V),
        dir (b) { }

    Dixel (const Voxel& V, const size_t b, const float l) :
        Voxel (V, l),
        dir (b) { }

    void set_dir   (const size_t b) { dir = b; }

    bool   valid()     const { return (dir != invalid); }
    size_t get_dir()   const { return dir; }

    Dixel& operator=  (const Dixel& V)       { Voxel::operator= (V); dir = V.dir; return *this; }
    Dixel& operator=  (const Voxel& V)       { Voxel::operator= (V); dir = invalid; return *this; }
    bool   operator== (const Dixel& V) const { return (Voxel::operator== (V) ? (dir == V.dir) : false); }
    bool   operator<  (const Dixel& V) const { return (Voxel::operator== (V) ? (dir <  V.dir) : Voxel::operator< (V)); }
    void   operator+= (const float l)  const { Voxel::operator+= (l); }

  private:
    size_t dir;

    static const size_t invalid;

};



// TODO TOD class: Would prefer the aPSF generation to be multi-threaded, so store the
//   SH coefficients in the voxel class
// Provide a normalise() function to remove any length dependence, and have unary contribution per streamline






class SetVoxelExtras
{
  public:
    float factor; // For TWI, when contribution to the map is uniform along the length of the track
    size_t index; // Index of the track
    float weight; // Cross-sectional multiplier for the track
};






// New classes that give sensible behaviour to the insert() function depending on the base class

class SetVoxel : public std::set<Voxel>, public SetVoxelExtras
{
  public:
    void insert (const Voxel& v)
    {
      std::set<Voxel>::insert (v);
    }
    void insert (const Voxel& v, const float l)
    {
      iterator existing = std::set<Voxel>::find (v);
      if (existing == std::set<Voxel>::end())
        std::set<Voxel>::insert (v);
      else
        (*existing) += l;
    }
};
class SetVoxelDEC : public std::set<VoxelDEC>, public SetVoxelExtras
{
  public:
    void insert (const VoxelDEC& v)
    {
      std::set<VoxelDEC>::insert (v);
    }
    void insert (const Voxel& v, const Point<float>& d)
    {
      iterator existing = std::set<VoxelDEC>::find (v);
      if (existing == std::set<VoxelDEC>::end()) {
        VoxelDEC temp (v, d);
        std::set<VoxelDEC>::insert (temp);
      } else {
        (*existing) += d;
      }
    }
    void insert (const Voxel& v, const Point<float>& d, const float l)
    {
      iterator existing = std::set<VoxelDEC>::find (v);
      if (existing == std::set<VoxelDEC>::end()) {
        VoxelDEC temp (v, d, l);
        std::set<VoxelDEC>::insert (temp);
      } else {
        existing->add (d, l);
      }
    }
};
class SetDixel : public std::set<Dixel>, public SetVoxelExtras
{
  public:
    void insert (const Dixel& v)
    {
      iterator existing = std::set<Dixel>::find (v);
      if (existing == std::set<Dixel>::end()) {
        std::set<Dixel>::insert (v);
      } else {
        (*existing) += 1.0f;
      }
    }
    void insert (const Voxel& v, const size_t d)
    {
      const Dixel temp (v, d);
      insert (temp);
    }
    void insert (const Voxel& v, const size_t d, const float l)
    {
      const Dixel temp (v, d, l);
      iterator existing = std::set<Dixel>::find (temp);
      if (existing == std::set<Dixel>::end()) {
        std::set<Dixel>::insert (temp);
      } else {
        (*existing) += l;
      }
    }
};




}
}
}
}

#endif



