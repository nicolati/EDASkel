// A remedial design database and cell library, part of EDASkel, a sample EDA app
// Copyright (C) 2010 Jeffrey Elliot Trull <linmodemstudent@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef SIMPLEDB_H
#define SIMPLEDB_H

// the idea here is to provide a simple database that conforms to the "Concept" expected
// by other parts of EDASkel.  It won't be fast, but should be easy to understand.
// Wrappers for "real" design databases are preferable for production work.

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <boost/assert.hpp>
#include <boost/optional.hpp>
// using the TR1, not Boost, versions of things wherever possible
// (I like Boost, but standards are even better)
#include <tr1/memory>

#include "globals.h"
using namespace EDASkel;

namespace SimpleDB {

  // helper functions
  template<class Obj>
    typename std::tr1::shared_ptr<Obj> findNamed(const std::string& name,
						 const std::vector<std::tr1::shared_ptr<Obj> >& vec) {
    typedef std::tr1::shared_ptr<Obj> Ptr;
    // linear search for a matching name
    // could make this Phoenix or Lambda or a complex bind expression.  But should I?
    for (typename std::vector<Ptr>::const_iterator oit = vec.begin();
	 oit != vec.end(); ++oit) {
      if ((*oit)->getName() == name)
	return *oit;
    }
    return Ptr(); // the shared_ptr equivalent of NULL
  }
      
  class LibCell {
  public:
    LibCell(const std::string& name) : m_name(name) {}
    std::string getName() const { return m_name; }

    void setDimensions(float w, float h) {m_dimensions = std::make_pair(w, h);}
    bool hasDimensions() const {return m_dimensions;}
    // BOOST_ASSERT that dimensions are present (i.e., caller has checked hasDimensions)
    // Boost's asserts can be easily turned off globally - yet another thing I don't have to invent
    float getWidth() const {BOOST_ASSERT(m_dimensions); return m_dimensions->first;}
    float getHeight() const {BOOST_ASSERT(m_dimensions); return m_dimensions->second;}

    void setClass(SiteClass c) { m_class = c; }
    SiteClass getClass() const { return m_class; }

    void setOrigin(const std::pair<float, float>& o) { m_origin = o; }
    std::pair<float, float> getOrigin() const { return m_origin; }

    void setSymmetry(const std::vector<SiteSymmetry>& ss) { m_symmetry = ss; }
    std::vector<SiteSymmetry> getSymmetry() const { return m_symmetry; }

    void setSite(const std::string& sitename) { m_site = sitename; }
    bool hasSite() { return m_site; }
    std::string getSite() { BOOST_ASSERT(m_site); return *m_site; }

  private:
    std::string m_name;
    SiteClass m_class;
    std::pair<float, float> m_origin;
    // I like to use boost::optional because it packages the idea of "data not present" cleanly
    // We may have taken this cell from a library that doesn't have physical dimensions for the cell
    // (e.g. a timing or power library) or it may not have been specified for some reason
    boost::optional<std::pair<float, float> > m_dimensions;
    // if no symmetry is supplied at all, placer tries "N" only
    // also tries FS if X, FN if Y, all the above plus S if X Y
    std::vector<SiteSymmetry> m_symmetry;   // maybe some kind of bitset here?
    boost::optional<std::string> m_site;
    
  };

  class LibSite {
  public:
    LibSite(const std::string& name) : m_name(name) {}
    std::string getName() const { return m_name; }

    void setClass(SiteClass c) { m_class = c; }
    SiteClass getClass() const { return m_class; }

    void setSymmetry(std::vector<SiteSymmetry> ss) { m_symmetry = ss; }
    bool hasSymmetry() const { return !m_symmetry.empty(); }
    std::vector<SiteSymmetry> getSymmetry() const { return m_symmetry; }

    void setDimensions(float w, float h) {m_dimensions = std::make_pair(w, h);}
    float getWidth() const {return m_dimensions.first;}
    float getHeight() const {return m_dimensions.second;}

  private:
    std::string m_name;
    SiteClass m_class;
    std::vector<SiteSymmetry> m_symmetry;
    std::pair<float, float> m_dimensions;
  };

  class Library {
  public:
    typedef LibCell Cell;
    typedef std::tr1::shared_ptr<Cell> CellPtr;
    // should Sites be here or in some sort of "tech" database?
    typedef LibSite Site;
    typedef std::tr1::shared_ptr<Site> SitePtr;
    Library() {}

    void addCell(CellPtr cell) {
      m_cells.push_back(cell);
    }

    CellPtr findCell(const std::string& name) const {
      return findNamed(name, m_cells);
    }

    CellPtr removeCell(CellPtr cell) {
      // erase-remove idiom
      m_cells.erase(std::remove(m_cells.begin(), m_cells.end(), cell), m_cells.end());
    }

    void addSite(SitePtr site) {
      m_sites.push_back(site);
    }

    SitePtr findSite(const std::string& name) const {
      return findNamed(name, m_sites);
    }

    SitePtr removeSite(SitePtr site) {
      m_sites.erase(std::remove(m_sites.begin(), m_sites.end(), site), m_sites.end());
    }

  private:
    std::vector<CellPtr> m_cells;
    std::vector<SitePtr> m_sites;
  };

  class DesPoint {
  public:
  DesPoint(int x, int y) : m_x(x), m_y(y) {}
    int x() const {return m_x;}
    int y() const {return m_y;}
  private:
    int m_x, m_y;
  };
    

  class DesRect {
  public:
  DesRect(DesPoint ll, DesPoint ur) : m_ll(ll), m_ur(ur) {}
    DesPoint ll() const {return m_ll;}
    DesPoint ur() const {return m_ur;}
  private:
    DesPoint m_ll, m_ur;
  };

  class Instance {
    // optional placement information
    struct InstPlacement {
      InstPlacement(const DesPoint& o, const std::string& ori, bool f) : origin(o), orient(ori), fixed(f) {}
      DesPoint origin;
      std::string orient;
      bool fixed;
    };

  public:
  Instance(const std::string& name, const std::string& cname) : m_name(name), m_cname(cname) {}
    // ultimately this will be a pointer to a library cell
    std::string getCellName() const {return m_cname;}

    std::string getName() const {return m_name;}

    // Because placement is optional and it's possible to set inconsistently,
    // I have only one "setter" here and three getters
    void setPlacement(DesPoint origin, const std::string& orient, bool fixed = false) {
      m_placement = InstPlacement(origin, orient, fixed);
    }
    bool hasPlacement() const { return m_placement; }
    DesPoint getOrigin() const { BOOST_ASSERT( m_placement ); return m_placement->origin; }
    std::string getOrient() const { BOOST_ASSERT( m_placement ); return m_placement->orient; }
    // if it's not even placed, then it's definitely not "fixed" either...
    bool isFixed() const { return (m_placement && m_placement->fixed); }
    
  private:
    std::string m_name, m_cname;
    boost::optional<InstPlacement> m_placement;
  };

  // a row/site array instantiation 
  // expand as needed (to save space)
  enum RowDir { ROWDIR_HORZ, ROWDIR_VERT };
  class RowSiteInst {
  public:
    RowSiteInst(const std::string& rowname, const std::string& sitename,
		RowDir dir, int count,
		const DesPoint& start, int step) :
      m_sitename(sitename), m_rowname(rowname),
      m_dir(dir), m_count(count), m_startloc(start), m_step(step) {}

    std::string getSiteName() const { return m_sitename; }
    std::string getRowName() const { return m_rowname; }
    RowDir      getDir() const { return m_dir; }
    int         getCount() const { return m_count; }
    DesPoint    getStartLoc() const { return m_startloc; }
    int         getStep() const { return m_step; }

  private:
    std::string m_sitename;   // better would be a pointer to a library site
    std::string m_rowname;
    Orient m_orient;
    RowDir m_dir;
    int m_count;              // count of sites in the direction (horz/vert) of the row
    DesPoint m_startloc;
    int m_step;               // dbu per step in direction of travel
  };

  class Database {
  public:
    typedef DesPoint Point;
    typedef DesRect Rect;
    typedef Instance Inst;
    typedef std::tr1::shared_ptr<Instance> InstPtr;
    typedef std::vector<InstPtr>::const_iterator InstIter;  // users cannot change stored pointers

    Database() {};
    std::pair<InstIter, InstIter> getInstances() const { return std::make_pair(m_instances.begin(),
									       m_instances.end()); }
    void setExtent(DesRect r) {m_extent = r;}
    bool hasExtent() const {return m_extent;}
    DesRect getExtent() const {BOOST_ASSERT(m_extent); return *m_extent;}

    void addInst(InstPtr i) {
      m_instances.push_back(i);
    }

    InstPtr findInst(const std::string& name) {
      return findNamed(name, m_instances);
    }

    void removeInst(InstPtr inst) {
      m_instances.erase(std::remove(m_instances.begin(), m_instances.end(), inst), m_instances.end());
    }

    void addRow(const std::string& name, const std::string& sitename,
		int xcount, int ycount,
		const DesPoint& start, int xstep, int ystep) {
      // translate information supplied into more compressed version with RowDir etc.
      BOOST_ASSERT( (xcount == 1) || (ycount == 1) );
      if (xcount == 1)
	m_rows.push_back(RowSiteInst(name, sitename, ROWDIR_VERT, ycount, start, ystep));
      else
	m_rows.push_back(RowSiteInst(name, sitename, ROWDIR_HORZ, xcount, start, xstep));
    }

    boost::optional<std::string> siteAt(int x, int y) {
      // supply the name of a site at the given location, if there is one
      // if we had access to the library we could do better checks
      // however it's a tricky architectural decision whether to have the db
      // "know" about the library.  Deferring this decision for now.
      for (std::vector<RowSiteInst>::const_iterator sit = m_rows.begin();
	   sit != m_rows.end(); ++sit) {
	int sx = sit->getStartLoc().x();
	int sy = sit->getStartLoc().y();
	for (int ct = 0; ct < sit->getCount(); ++ct) {
	  if ((x == sx) && (y == sy))
	    // found it!
	    return sit->getSiteName();
	  if (sit->getDir() == ROWDIR_HORZ)
	    sx += sit->getStep();
	  else
	    sy += sit->getStep();
	}
      }

      return boost::optional<std::string>();
    }

    void setDbuPerMicron(int dbu) { m_dbupermicron = dbu; }
    int getDbuPerMicron() const { return m_dbupermicron; }

  private:
    std::vector<InstPtr> m_instances;
    boost::optional<DesRect> m_extent;
    std::vector<RowSiteInst> m_rows;
    int m_dbupermicron;
  };

}    

#endif
