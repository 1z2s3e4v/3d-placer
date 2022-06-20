// 2006-03-01 Tien Chang Hsu

#include "placedb.h"
#include "cellmoving.h"



#include <vector>
using namespace std;
void CCellmovingRowSegment::printRow(const double &cutLine)
{
    map < double, int >::iterator iter;
    int size = this->m_rowmodule.size();

    cout << "\n====print row====CUTLINE:" << cutLine << " Size:" << size <<
        "\n";
    for (iter = m_rowmodule.begin(); iter != m_rowmodule.end(); ++iter)
    {
        cerr << "[" << iter->first << "," << iter->second << "] ";
    }
}

void CCellmovingRowSegment::printRow(CPlaceDB & db, const double &cutLine)
{
    map < double, int >::iterator iter;
    int size = this->m_rowmodule.size();

    cout << "\n====print row====CUTLINE:" << cutLine << " Size:" << size <<
        "\n";
    for (iter = m_rowmodule.begin(); iter != m_rowmodule.end(); ++iter)
    {
        cerr << "[" << iter->first << "," << iter->second << ", mx:" << db.
            m_modules[iter->second].m_x << "] ";
    }
}
void CCellmovingRowSegment::updateRowModule(const double &cutLine,
                                            CPlaceDB & db, bool isRight,
                                            const double &x,
                                            const double &length)
{
    if (isRight == true)               //shift module to right side
    {
        map < double, int >newmap;
        set < double >eraseSet;
        double newx = cutLine - length;
        map < double, int >::iterator iter = m_rowmodule.find(x);

        assert(iter != m_rowmodule.end());      //the x location must exist in m_rowmodule!!
        assert(iter->first == x);
//              assert(db.m_modules[iter->second].m_x==iter->first);

        newmap[newx] = iter->second;
        eraseSet.insert(iter->first);
        db.SetModuleLocation(iter->second, newx,
                             db.m_modules[iter->second].m_y);
        bool stop = false;
        double xFlag = newx + db.m_modules[iter->second].m_width;

        while (stop == false)
        {
            ++iter;
            if (iter == m_rowmodule.end())
            {
                stop = true;
            } else
            {
                if (db.m_modules[iter->second].m_x < xFlag)
                {
                    //                      assert(iter->first==db.m_modules[iter->second].m_x);

                    eraseSet.insert(iter->first);
                    newx = xFlag;
                    newmap[newx] = iter->second;
                    eraseSet.insert(iter->first);
                    xFlag += db.m_modules[iter->second].m_width;
                    db.SetModuleLocation(iter->second, newx,
                                         db.m_modules[iter->second].m_y);
                } else
                {
                    stop = true;
                }
            }
        }

        //////////////////////////////
        //update m_rowmodule
        ///////////////////////////////
        set < double >::iterator eit;

        for (eit = eraseSet.begin(); eit != eraseSet.end(); ++eit)
        {
            m_rowmodule.erase(*eit);
        }
        map < double, int >::iterator nit;

        for (nit = newmap.begin(); nit != newmap.end(); ++nit)
        {
            m_rowmodule[nit->first] = nit->second;
        }
    } else                             //shift module to left side
    {
        map < double, int >newmap;
        set < double >eraseSet;

        map < double, int >::iterator iter = m_rowmodule.find(x);

        assert(iter != m_rowmodule.end());      //the x location must exist in m_rowmodule!!
//              assert(iter->first==x);
//              assert(db.m_modules[iter->second].m_x==iter->first);

        double newx =
            cutLine - (db.m_modules[iter->second].m_width - length);
        newmap[newx] = iter->second;
        eraseSet.insert(iter->first);
        db.SetModuleLocation(iter->second, newx,
                             db.m_modules[iter->second].m_y);
        bool stop = false;
        double xFlag = newx;

        if (iter != m_rowmodule.begin())
        {
            while (stop == false)
            {
                --iter;
                if (iter == m_rowmodule.begin())
                {
                    if ((db.m_modules[iter->second].m_x +
                         db.m_modules[iter->second].m_width) > xFlag)
                    {
                        //                              assert(iter->first==db.m_modules[iter->second].m_x);

                        eraseSet.insert(iter->first);
                        newx = xFlag - db.m_modules[iter->second].m_width;
                        newmap[newx] = iter->second;
                        eraseSet.insert(iter->first);
                        xFlag -= db.m_modules[iter->second].m_width;
                        db.SetModuleLocation(iter->second, newx,
                                             db.m_modules[iter->second].
                                             m_y);
                        assert(newx >= m_absXstart);
                    }
                    stop = true;
                } else
                {
                    if (iter->second >= (int) db.m_modules.size())
                    {
                        this->printRow(db, cutLine);
                        cerr << "Warning:: " << iter->
                            first << " second: " << iter->second << "\n";
                    }
                    if ((db.m_modules[iter->second].m_x +
                         db.m_modules[iter->second].m_width) > xFlag)
                    {
                        //                              assert(iter->first==db.m_modules[iter->second].m_x);

                        eraseSet.insert(iter->first);
                        newx = xFlag - db.m_modules[iter->second].m_width;
                        newmap[newx] = iter->second;
                        eraseSet.insert(iter->first);
                        xFlag -= db.m_modules[iter->second].m_width;
                        db.SetModuleLocation(iter->second, newx,
                                             db.m_modules[iter->second].
                                             m_y);
                    } else
                    {
                        stop = true;
                    }
                }
            }
        }
        //////////////////////////////
        //update m_rowmodule
        ///////////////////////////////
        set < double >::iterator eit;

        for (eit = eraseSet.begin(); eit != eraseSet.end(); ++eit)
        {
            m_rowmodule.erase(*eit);
        }
        map < double, int >::iterator nit;

        for (nit = newmap.begin(); nit != newmap.end(); ++nit)
        {
            m_rowmodule[nit->first] = nit->second;
        }
    }
}

void CCellmovingRowSegment::findFirstModule(const double &cutLine,
                                            CPlaceDB & db, bool isRight,
                                            double &step, double &x,
                                            double &length)
{
    if (isRight == true)               //find the first module to move in the LEFT side of the cutLine
    {
        double res = step;
        map < double, int >::iterator iter =
            m_rowmodule.lower_bound(cutLine);
        if (iter == m_rowmodule.begin())        //no movable module left to cutLine; no modules can be moved!!
        {
            step = 0;
            return;
        } else
        {
            --iter;
            bool stop = false;

            if ((db.m_modules[iter->second].m_x +
                 db.m_modules[iter->second].m_width) >= cutLine)
            {
                if ((cutLine - db.m_modules[iter->second].m_x) >= step)
                {
                    //if(db.m_modules[iter->second].m_x!=iter->first)
                    //{
                    //      cerr<<"\n mx:"<<db.m_modules[iter->second].m_x<<" iter:"<<iter->first;
                    //      this->printRow(db,cutLine);
                    //      cerr<<"\n";

                    //}
                    //              assert(db.m_modules[iter->second].m_x==iter->first);
//                                      x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length =
                        cutLine - db.m_modules[iter->second].m_x - step;
                    return;
                } else
                {
                    res -= cutLine - db.m_modules[iter->second].m_x;
//                                      x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length = 0;

                }
            } else
            {
                if (db.m_modules[iter->second].m_width >= step)
                {
                    //      x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length = db.m_modules[iter->second].m_width - step;
                    return;
                } else
                {
                    res -= db.m_modules[iter->second].m_width;
//                                      x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length = 0;
                }
            }
            while (stop == false)
            {
                if (iter == m_rowmodule.begin())
                {
                    step = step - res;
                    return;
                } else
                {
                    --iter;
                    if (db.m_modules[iter->second].m_width >= res)
                    {
//                                              x=db.m_modules[iter->second].m_x;
                        x = iter->first;
                        length = db.m_modules[iter->second].m_width - res;
                        return;
                    } else
                    {
                        res -= db.m_modules[iter->second].m_width;
                        //x=db.m_modules[iter->second].m_x;
                        x = iter->first;
                        length = 0;
                    }
                }
            }
        }
    } else                             //find the first module to move in the RIGHT side of the cutLine
    {
        double res = step;
        map < double, int >::iterator iter =
            m_rowmodule.lower_bound(cutLine);
        if (iter != m_rowmodule.begin())
        {
            --iter;
            if ((db.m_modules[iter->second].m_x +
                 db.m_modules[iter->second].m_width) > cutLine)
            {
                if (((db.m_modules[iter->second].m_x +
                      db.m_modules[iter->second].m_width) - cutLine) >=
                    step)
                {
                    //x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length =
                        (db.m_modules[iter->second].m_x +
                         db.m_modules[iter->second].m_width) - cutLine -
                        step;
                    return;

                } else
                {
                    res -=
                        ((db.m_modules[iter->second].m_x +
                          db.m_modules[iter->second].m_width) - cutLine);
                    //x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length = 0;
                }
            }
            ++iter;
        }


        bool stop = false;

        while (stop == false)
        {
            if (iter == m_rowmodule.end())
            {
                step = step - res;
                return;
            } else
            {
                if (db.m_modules[iter->second].m_width >= res)
                {
                    //x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length = db.m_modules[iter->second].m_width - res;
                    return;
                } else
                {
                    res -= db.m_modules[iter->second].m_width;
                    //x=db.m_modules[iter->second].m_x;
                    x = iter->first;
                    length = 0;
                }
            }
            ++iter;
        }

    }
}
void CCellmovingRowSegment::shiftCell(const double &cutLine, CPlaceDB & db,
                                      bool isRight, double &step)
{
    double x = 0;
    double length = 0;

    findFirstModule(cutLine, db, isRight, step, x, length);
    if (step != 0)
    {
        updateRowModule(cutLine, db, isRight, x, length);
    }

}
void CCellmovingRowSegment::findEmptySpace(const double &cutLine,
                                           CPlaceDB & db, bool isRight,
                                           double &empty)
{
    empty = 0;
    double moduleLength = 0;

    if (m_rowmodule.size() == 0)
    {
        return;
    }

    if (isRight == true)               //find the empty space in the right side of cutLine
    {
        if ((this->m_absXstart + this->m_length) <= cutLine)
        {
            return;                    //empty=0;
        }

        map < double, int >::iterator iter =
            m_rowmodule.upper_bound(cutLine);
        if (iter == m_rowmodule.end())
        {
            if (iter != m_rowmodule.begin())
            {
                --iter;
                if (iter->second >= (int) db.m_modules.size())
                {
                    this->printRow(cutLine);
                    cerr << "Warning:: " << iter->
                        first << " second: " << iter->second << "\n";
                }

                if ((db.m_modules[iter->second].m_x +
                     db.m_modules[iter->second].m_width) > cutLine)
                {
                    moduleLength +=
                        (db.m_modules[iter->second].m_x +
                         db.m_modules[iter->second].m_width) - cutLine;
                }
                ++iter;
            }
            empty =
                (this->m_absXstart + this->m_length) - cutLine -
                moduleLength;
            return;
        }

        if (iter != m_rowmodule.begin())
        {
            --iter;
            if (iter->second >= (int) db.m_modules.size())
            {
                cerr << "Warning::" << iter->second;
            }

            if ((db.m_modules[iter->second].m_x +
                 db.m_modules[iter->second].m_width) > cutLine)
            {
                moduleLength +=
                    (db.m_modules[iter->second].m_x +
                     db.m_modules[iter->second].m_width) - cutLine;
            }
            ++iter;
        }

        for (; iter != m_rowmodule.end(); ++iter)
        {
            moduleLength += db.m_modules[iter->second].m_width;
        }
        empty =
            (this->m_absXstart + this->m_length) - cutLine - moduleLength;

        if (cutLine < this->m_absXstart)
        {
            empty -= m_absXstart - cutLine;
        }
        assert(empty >= 0);
        return;

    } else                             //find the empty space in the left side of cutLine
    {
        if (m_absXstart >= cutLine)
        {
            return;                    //empty=0;
        }
        map < double, int >::iterator iter;

        for (iter = m_rowmodule.begin();
             iter != m_rowmodule.lower_bound(cutLine); ++iter)
        {
            if ((db.m_modules[iter->second].m_x +
                 db.m_modules[iter->second].m_width) <= cutLine)
            {
                moduleLength += db.m_modules[iter->second].m_width;
            } else
            {
                moduleLength += cutLine - db.m_modules[iter->second].m_x;
                break;
            }
        }
        empty = cutLine - m_absXstart - moduleLength;
        if (cutLine > (m_absXstart + m_length))
        {
            empty -= cutLine - (m_absXstart + m_length);
        }
        assert(empty >= 0);
        return;
    }
}

bool CCellmovingRowSegment::insert_module(CPlaceDB & db, int mID)
{
    assert((int) db.m_modules.size() > mID);
    assert(db.m_modules[mID].m_x >= this->m_absXstart);
    if (!(db.m_modules[mID].m_x <= (this->m_absXstart + this->m_length)))
    {
        cerr << "\n mID:" << mID << " x:" << db.m_modules[mID].
            m_x << " mabsXst:" << m_absXstart << " length:" << m_length <<
            " +value" << (this->m_absXstart + this->m_length) << endl;
        fprintf(stderr, "module y: %f\n", db.m_modules[mID].m_y);
        flush(cerr);
    }
    assert(db.m_modules[mID].m_x <= (this->m_absXstart + this->m_length));
    assert(m_rowmodule.find(db.m_modules[mID].m_x) == m_rowmodule.end());
    m_rowmodule[db.m_modules[mID].m_x] = mID;
    return true;
}

double CCellmoving::redistributeCellUnitRun(double targetUtil,
                                            bool isGoRight)
{
//      double time1=seconds();
    //////////////////////////////////////////////
    // Init
    /////////////////////////////////////////////

    //double wl1,wl2;
    //m_pDB->CalcHPWL();
    //wl1=m_pDB->GetHPWLp2p();
    CPlaceBin placeBin(*m_pDB);

    placeBin.CreateGrid(m_pDB->m_rowHeight * 10.0, m_pDB->m_rowHeight * 10.0);
    //double penalty1 = placeBin.GetPenalty( targetUtil );
    //wl1=wl1*(1+penalty1/100.0);
    //if(showMsg==true)
    //{
    //      cout<<"\n%%%%%%%%%%%%%%%%%% Start Cell Redistribution with target:"<<targetUtil<<"\n";
    //      m_pDB->OutputGnuplotFigureWithZoom("beforeCR",false,true,true);
    //      placeBin.ShowInfo( targetUtil );
    //}




    int num_rows = (int) m_pDB->m_sites.size();

    //double botto;
    //cout<<"\n Rows:"<<num_rows<<'\n';
    //float row_length = m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left;

    //////////////////////////////////////////////
    // initailize m_coreRows
    //////////////////////////////////////////////
    m_coreRows.clear();
    m_coreRows.resize(num_rows);
    m_coreRowsMap.clear();
    m_coreRowsMap.resize(num_rows);

    // build row empty map
    for (unsigned int i = 0; i < m_pDB->m_sites.size(); i++)
    {
        int pos =
            (int) ((m_pDB->m_sites[i].m_bottom -
                    m_pDB->m_coreRgn.bottom) / m_pDB->m_rowHeight);
        assert(pos < (int) m_coreRows.size());
        m_coreRows[pos].reserve(m_pDB->m_sites[i].m_interval.size() / 2);
        for (unsigned int j = 0; j < m_pDB->m_sites[i].m_interval.size();
             j = j + 2)
        {
            double length =
                m_pDB->m_sites[i].m_interval[j + 1] -
                m_pDB->m_sites[i].m_interval[j];
            m_coreRows[pos].
                push_back(CCellmovingRowSegment
                          (m_pDB->m_sites[i].m_interval[j], length));
            m_coreRowsMap[pos][m_pDB->m_sites[i].m_interval[j]] =
                m_coreRows[pos].size() - 1;
        }
    }
    //===================================================
    // Put blocks into m_coreRows
    //===================================================
    for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
    {

        //only insert movable standard cell into datastructure
        if ((m_pDB->m_modules[i].m_isFixed == false)
            && (m_pDB->m_modules[i].m_height == m_pDB->m_rowHeight))
        {
            insertModule(i);
        } else if (m_pDB->m_modules[i].m_isFixed == false)
        {
            //              cerr<<"\nWarning: module:"<<i<<" movable macro exists!!";
        }
    }

    //if(showMsg==true)
    //{
    //      string fn="density_before.dat";
    //      OutputDensityGrid( targetUtil,fn,placeBin );
    //      cout<<"\nStart Shift Cell";
    //}


    if (isGoRight == true)
    {
        //===================================================
        // Start shift cell: 1. from left to right
        //===================================================
        for (int i = 0; i < placeBin.m_binNumberH; i++)
        {
            for (int j = 0; j < placeBin.m_binNumberW - 1; j++)
            {
                double flux =
                    findFlux(targetUtil, j, i, j + 1, i, placeBin);
                if (flux != 0)
                {
                    moveCell(flux, j, i, j + 1, i, placeBin);
                }
            }
        }
    } else
    {
        //===================================================
        // Start shift cell: 2. from right to left
        //===================================================
        for (int i = 0; i < placeBin.m_binNumberH; i++)
        {
            for (int j = placeBin.m_binNumberW - 1; j > 0; --j)
            {
                double flux =
                    findFlux(targetUtil, j, i, j - 1, i, placeBin);
                if (flux != 0)
                {
                    moveCell(flux, j, i, j - 1, i, placeBin);
                }
            }
        }
    }

    //m_pDB->Align();
    placeBin.UpdateBinUsage();
    double penalty2 = placeBin.GetPenalty(targetUtil);

    m_pDB->CalcHPWL();
    double wl2 = m_pDB->GetHPWLp2p();
    double wl2p = wl2 * (1 + penalty2 / 100.0);

    if (showMsg == true)
    {
        if (isGoRight == true)
        {
//                      cout<<"\nRight move, HPWL="<<wl2<<", HPWL with Penalty="<<wl2p;
            printf
                ("\nRight move, penalty = %.2f \t(HPWL = %.0f)\t ,(HPWL with penalty = %.0f)",
                 penalty2, wl2, wl2p);
        } else
        {
            printf
                ("\nLeft  move, penalty = %.2f \t(HPWL = %.0f)\t ,(HPWL with penalty = %.0f)",
                 penalty2, wl2, wl2p);
        }
    }
    return wl2p;
}
void CCellmoving::redistributeCell(double targetUtil)
{
    double time1 = seconds();

    //////////////////////////////////////////////
    // Init
    /////////////////////////////////////////////
    bool showMsg = true;

    m_pDB->CalcHPWL();
    double wl1 = m_pDB->GetHPWLp2p();
    CPlaceBin placeBin(*m_pDB);

    placeBin.CreateGrid(m_pDB->m_rowHeight * 10.0, m_pDB->m_rowHeight * 10.0);
    double penalty = placeBin.GetPenalty(targetUtil);
    double wl1p = wl1 * (1 + penalty / 100.0);
    double wlstart = wl1p;

    m_pDB->SaveBlockLocation();

    if (showMsg == true)
    {
        cout <<
            "\n%%%%%%%%%%%%%%%%%% Start cell redistribution with target: "
            << targetUtil;
        printf
            ("\nInitial State: penalty = %.2f \t(HPWL = %.0f)\t ,(HPWL with penalty = %.0f)\n",
             penalty, wl1, wl1p);
#if 0                                  // donnie
        m_pDB->OutputGnuplotFigureWithZoom("beforeCR", false, true, true);
        //placeBin.ShowInfo( targetUtil );
        string fn = "density_before.dat";

        OutputDensityGrid(targetUtil, fn, placeBin);
        //      cout<<"\nStart Shift Cell";
#endif
    }

    bool stop = false;
    int runCount = 0;
    int incCount = 0;

    while (stop == false)
    {
        bool goRight = true;

        if (runCount % 2 == 1)
        {
            goRight = false;
        }
        double newWlp = redistributeCellUnitRun(targetUtil, goRight);

        if (newWlp < (wl1p * 0.99999))
        {
            wl1p = newWlp;
            m_pDB->SaveBlockLocation();
            incCount = 0;
            if (showMsg == true)
                cout << " *";
            //cout << "Save Best!!";
        } else
        {
            ++incCount;
        }
        if (incCount > 3)
        {
            stop = true;
        }
        ++runCount;

    }

    m_pDB->RestoreBlockLocation();
//      m_pDB->Align();
    if (showMsg == true)
    {
        cout << "\nAfter cell redistribution";
        m_pDB->CalcHPWL();

        CPlaceBin placeBin2(*m_pDB);

        placeBin2.CreateGrid(m_pDB->m_rowHeight * 10.0, m_pDB->m_rowHeight * 10.0);
        double penalty2 = placeBin2.GetPenalty(targetUtil);

        double wl2 = m_pDB->GetHPWLp2p();
        double wl2p = wl2 * (1 + penalty2 / 100.0);

#if 0                                  // donnie
        string fn = "density_after.dat";

        OutputDensityGrid(targetUtil, fn, placeBin2);

        m_pDB->OutputGnuplotFigureWithZoom("afterCR", false, true, true);
#endif

//              m_pDB->OutputPL("AFTER_CR.pl");
        printf
            ("\nCR Final: penalty = %.2f \t(HPWL = %.0f)\t ,(HPWL with penalty = %.0f)",
             penalty2, wl2, wl2p);
        printf
            ("\n%%%%%%%%%%%%%%%%%% Finish Cell Redistribution, improve %.2f%%, total time: %.2f sec\n",
             100 - (wl2p / wlstart) * 100, seconds() - time1);


        /* move to main.cpp by donnie
           //check overlapping
           CCheckLegal clegal(*m_pDB);
           clegal.check();
         */
    }

}

//void CCellmoving::redistributeCell(double targetUtil)
//{
//      double time1=seconds();
//      //////////////////////////////////////////////
//      // Init
//      /////////////////////////////////////////////
//      bool showMsg=true;
//      double wl1,wl2;
//      m_pDB->CalcHPWL();
//      wl1=m_pDB->GetHPWLp2p();
//    CPlaceBin placeBin( *m_pDB );
//    placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0 );
//
//      if(showMsg==true)
//      {
//              cout<<"\n%%%%%%%%%%%%%%%%%% Start Cell Redistribution with target:"<<targetUtil<<"\n";
//              m_pDB->OutputGnuplotFigureWithZoom("beforeCR",false,true,true);
//              placeBin.ShowInfo( targetUtil );
//      }
//
//
//
//
//    int num_rows = (int)m_pDB->m_sites.size();
//      //double botto;
//    //cout<<"\n Rows:"<<num_rows<<'\n';
//    //float row_length = m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left;
//
//      //////////////////////////////////////////////
//    // initailize m_coreRows
//      //////////////////////////////////////////////
//    m_coreRows.clear();
//      m_coreRows.resize(num_rows);
//      m_coreRowsMap.clear();
//      m_coreRowsMap.resize(num_rows);
//
//    // build row empty map
//    for(unsigned int i=0;i<m_pDB->m_sites.size();i++)
//    {
//              int pos=(int)( (m_pDB->m_sites[i].m_bottom-m_pDB->m_coreRgn.bottom)/m_pDB->m_rowHeight);
//              assert(pos<(int)m_coreRows.size());
//              m_coreRows[pos].reserve(m_pDB->m_sites[i].m_interval.size()/2);
//              for(unsigned int j=0;j<m_pDB->m_sites[i].m_interval.size();j=j+2)
//              {
//                      double length=m_pDB->m_sites[i].m_interval[j+1]-m_pDB->m_sites[i].m_interval[j];
//                      m_coreRows[pos].push_back( CCellmovingRowSegment( m_pDB->m_sites[i].m_interval[j], length) );
//                      m_coreRowsMap[pos][m_pDB->m_sites[i].m_interval[j]]=m_coreRows[pos].size()-1;
//              }                       
//    }
//      //===================================================
//      // Put blocks into m_coreRows
//      //===================================================
//      for(unsigned  int i=0; i<m_pDB->m_modules.size(); i++ )
//      {
//
//              //only insert movable standard cell into datastructure
//              if((m_pDB->m_modules[i].m_isFixed==false) && (m_pDB->m_modules[i].m_height==m_pDB->m_rowHeight)) 
//          {
//                      insertModule(i);
//          }
//              else if(m_pDB->m_modules[i].m_isFixed==false)
//              {
//      //              cerr<<"\nWarning: module:"<<i<<" movable macro exists!!";
//              }
//      }
//
//      if(showMsg==true)
//      {
//              string fn="density_before.dat";
//              OutputDensityGrid( targetUtil,fn,placeBin );
//              cout<<"\nStart Shift Cell";
//      }
//
//      //===================================================
//      // Start shift cell: 1. from left to right
//      //===================================================
//    for( int i=0; i<placeBin.m_binNumberH; i++ )
//    {
//              for( int j=0; j<placeBin.m_binNumberW-1; j++ )
//              {
//                      double flux=findFlux(targetUtil,j,i,j+1,i,placeBin);
//                      if(flux!=0)
//                      {
//                              moveCell(flux,j,i,j+1,i,placeBin);
//                      }
//              }
//      }
//      //===================================================
//      // Start shift cell: 2. from right to left
//      //===================================================
//    for( int i=0; i<placeBin.m_binNumberH; i++ )
//    {
//              for( int j=placeBin.m_binNumberW-1; j>0; --j )
//              {
//                      double flux=findFlux(targetUtil,j,i,j-1,i,placeBin);
//                      if(flux!=0)
//                      {
//                              moveCell(flux,j,i,j-1,i,placeBin);
//                      }
//              }
//      }
//      if(showMsg==true)
//      {
//              cout<<"\nAfter cell redistribution";
//              m_pDB->CalcHPWL();
//              CPlaceBin placeBin2( *m_pDB );
//              placeBin2.CreateGrid( m_pDB->m_rowHeight * 10.0 );
//              placeBin2.ShowInfo( targetUtil );
//              string fn="density_after.dat";
//              OutputDensityGrid( targetUtil,fn,placeBin2 );
//
//
//              wl2=m_pDB->GetHPWLp2p();
//              cout<<"\nWL before:"<<wl1<<" WL after:"<<wl2<<"\n";
//
//              m_pDB->OutputGnuplotFigureWithZoom("afterCR",false,true,true);
//
//              //check overlapping
//              m_pDB->Align();
//              CCheckLegal clegal(*m_pDB);
//              clegal.check();
//              cout<<"\n%%%%%%%%%%%%%%%%%% Finish Cell Redistribution, total time:"<<seconds()-time1<<" seconds\n";
//
//      }
//
//}
void CCellmoving::insertModule(int mID)
{
    //////////////////////////////////
    // 1.find row and x position
    //////////////////////////////////
    double mx = m_pDB->m_modules[mID].m_x + 0.1;
    double my = m_pDB->m_modules[mID].m_y;
    int y_pos =
        (int) ((my - m_pDB->m_coreRgn.bottom) / m_pDB->m_rowHeight);
//      assert( (y_pos*m_pDB->m_rowHeight+m_pDB->m_coreRgn.bottom)==m_pDB->m_modules[mID].m_y); //the modules should be on the row
    assert(y_pos < (int) m_coreRows.size());
    int x_pos;
    map < double, int >::iterator it =
        m_coreRowsMap[y_pos].upper_bound(mx);

    if (it == m_coreRowsMap[y_pos].begin())
    {
        cerr << "\nWarning: module:" << mID <<
            " 's x coordinate are out of site!!";
    } else
    {
        --it;
        x_pos = it->second;
        if (!
            (m_pDB->m_modules[mID].m_x <=
             (m_coreRows[y_pos][x_pos].m_absXstart +
              m_coreRows[y_pos][x_pos].m_length)))
        {
            cerr << "\n mID:" << mID << " x:" << m_pDB->m_modules[mID].
                m_x << " mabsXst:" << m_coreRows[y_pos][x_pos].
                m_absXstart << " length:" << m_coreRows[y_pos][x_pos].
                m_length << " +value" << (m_coreRows[y_pos][x_pos].
                                          m_absXstart +
                                          m_coreRows[y_pos][x_pos].
                                          m_length) << endl;
            cerr << "ypos" << y_pos << " bottom:" << m_pDB->m_coreRgn.
                bottom << " rowHeight" << m_pDB->m_rowHeight << endl;
            fprintf(stderr, "module y: %f\n", m_pDB->m_modules[mID].m_y);


            flush(cerr);
        }

        m_coreRows[y_pos][x_pos].insert_module(*m_pDB, mID);


    }



}
double CCellmoving::findFlux(double target, int binX1, int binY1,
                             int binX2, int binY2, CPlaceBin & placeBin)
{
    if ((placeBin.m_binSpace[binX1][binY1] == 0)
        || (placeBin.m_binSpace[binX2][binY2] == 0))
    {
        return 0;
    } else
    {
        double targetUsage1 = placeBin.m_binSpace[binX1][binY1] * target;
        double targetUsage2 = placeBin.m_binSpace[binX2][binY2] * target;
        double util1 =
            placeBin.m_binUsage[binX1][binY1] /
            placeBin.m_binSpace[binX1][binY1];
        double util2 =
            placeBin.m_binUsage[binX2][binY2] /
            placeBin.m_binSpace[binX2][binY2];

        if (util1 <= target)
        {
            return 0;
        } else if (util1 <= util2)
        {
            return 0;
        } else                         //try to balance the util of bin1 and bin2
        {
            double residual =
                (placeBin.m_binUsage[binX1][binY1] - targetUsage1) +
                (placeBin.m_binUsage[binX2][binY2] - targetUsage2);
            if (residual <= 0)         //bin2 has enough space 
            {
                return placeBin.m_binUsage[binX1][binY1] - targetUsage1;

            }
            double idealUsage1 =
                targetUsage1 +
                residual * (placeBin.m_binSpace[binX1][binY1] /
                            (placeBin.m_binSpace[binX1][binY1] +
                             placeBin.m_binSpace[binX2][binY2]));
            return placeBin.m_binUsage[binX1][binY1] - idealUsage1;
        }

    }
}

//move flux area of cells from bin1 to bin2
void CCellmoving::moveCell(double flux, int binX1, int binY1, int binX2,
                           int binY2, CPlaceBin & placeBin)
{
    /////////////////////////////////////////////////////
    // STEP 1: find common rows (row segment between bin1 and bin2)
    /////////////////////////////////////////////////////
    double cutLine;
    bool isGoRight;

    if (binX2 > binX1)
    {
        isGoRight = true;
    } else
    {
        isGoRight = false;
    }
    int rowPosStart = findRowPos(placeBin.GetBinY(binY1));
    int rowPosEnd =
        findRowPos(placeBin.GetBinY(binY1) + placeBin.GetBinHeight());
    vector < int >interBinRowsID;
    vector < int >interBinRowsSegmentID;

    vector < bool > blackList;
    interBinRowsID.reserve(10);        //store common row ID
    interBinRowsSegmentID.reserve(10); //store common row segment ID
    blackList.reserve(10);             //store rows which has no empty space (in order to save time)
    for (int i = rowPosStart; i < rowPosEnd; i++)
    {


        if (isGoRight == true)
        {
            cutLine = placeBin.GetBinX(binX2);
        } else
        {
            cutLine = placeBin.GetBinX(binX1);
        }
        cutLine = ceil(cutLine);
        map < double, int >::iterator it;

        it = m_coreRowsMap[i].lower_bound(cutLine);
        if (it != m_coreRowsMap[i].begin())
        {
            --it;
            if ((it->first < cutLine)
                && ((m_coreRows[i][it->second].m_length + it->first) >
                    cutLine))
            {

                interBinRowsID.push_back(i);    //store common row ID
                interBinRowsSegmentID.push_back(it->second);    //store common row ID
                blackList.push_back(false);
            }
        }
    }

    ///////////////////////////////////////////////////////
    // STEP 2: move cells in the common rows
    ///////////////////////////////////////////////////////
    bool stop = false;
    int runCount = 0;
    int stepRatio = 10;
    double totalAreaGain = 0;
    double step = flux / (m_pDB->m_rowHeight * stepRatio);

    while (stop == false)
    {
        for (int i = 0; i < (int) interBinRowsID.size(); i++)
        {
            if (blackList[i] == true)
                continue;

            double areaGain = 0;
            double s = step;

            if ((totalAreaGain + step * m_pDB->m_rowHeight) > flux)     //prevent over moving
            {
                s = floor((flux - totalAreaGain) / m_pDB->m_rowHeight);
            }
//                      assert(s>0);
            moveSegmentCell(s, interBinRowsID[i], interBinRowsSegmentID[i],
                            cutLine, isGoRight, areaGain);
            if (areaGain == 0)
            {
                blackList[i] = true;
            } else
            {
                totalAreaGain += areaGain;
            }
            if (totalAreaGain >= flux)
            {
                stop = true;
                break;
            }

        }
        ++runCount;
        if (runCount >= 4)
            stop = true;
    }
    placeBin.m_binUsage[binX1][binY1] -= totalAreaGain;
    placeBin.m_binUsage[binX2][binY2] += totalAreaGain;




}

void CCellmoving::moveSegmentCell(const double &targetStepXmove,
                                  const int &rowID, const int &segID,
                                  const double &cutLine,
                                  const bool & isGoRight, double &areaGain)
{

    if (targetStepXmove == 0)
    {
        areaGain = 0;
        return;
    }
    double empty = 0;

    m_coreRows[rowID][segID].findEmptySpace(cutLine, *m_pDB, isGoRight,
                                            empty);
    if (empty == 0)
    {
        areaGain = 0;
        return;
    } else
    {
        int realStep = (int) floor(min(targetStepXmove, empty));

//              double realStep=min(targetStepXmove,empty);
        if (realStep > 0)
        {
            double rs = (double) realStep;

            m_coreRows[rowID][segID].shiftCell(cutLine, *m_pDB, isGoRight,
                                               rs);
        }

        assert(realStep >= 0);

        areaGain = m_pDB->m_rowHeight * realStep;
        return;
    }

}

int CCellmoving::findRowPos(double y)
{
    int pos = (int) ((y - m_pDB->m_coreRgn.bottom) / m_pDB->m_rowHeight);

    if (pos >= (int) this->m_coreRows.size())
        return m_coreRows.size() - 1;
    else
        return pos;
}

void CCellmoving::OutputDensityGrid(double target, string filename,
                                    CPlaceBin & placeBin)
{
    //output density map to file
    ofstream out(filename.c_str());

    for (unsigned int i = 0; i < placeBin.m_binSpace[0].size(); i++)
    {
        for (unsigned int j = 0; j < placeBin.m_binSpace.size(); j++)
        {
            double density;

            if (placeBin.m_binSpace[j][i] == 0)
                density = 0;
            else
            {
                density =
                    placeBin.m_binUsage[j][i] / placeBin.m_binSpace[j][i] -
                    target;
                if (density < 0)
                    density = 0;
            }
            out << density << " ";
        }
        out << endl;
    }
    out.close();

}
