// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@i.cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

//---------------------------------------------------------------------------
#include <cmath>
#include <iostream>
#include <cassert>
#include <iomanip>

using namespace std;

#include "sa.h"
using namespace N_ntumptree;
//---------------------------------------------------------------------------
namespace N_ntumptree{
double init_avg = 0.00001;
int hill_climb_stage = 7;
double avg_ratio=150;
double lamda=1.3;
bool forgeFlag = false;

double P=0.9;


double mean(vector<double> &chain)
{
    double sum=0;
    for(int i=0; i < (int)chain.size();i++)
	sum+= chain[i];
    return sum/chain.size();
}

double var(vector<double> &chain)
{
    double m = mean(chain);
    double sum=0;
    double var;
    int N= chain.size();

    for(int i=0; i < N;i++)
	sum += (chain[i]-m)*(chain[i]-m);

    var = sum/(N);
    //printf("  m=%.4f ,v=%.4f\n",m,var);

    return var;
}

/* Simulated Annealing B*Tree Floorplan
   k: factor of the number of permutation in one temperature
   local: local search iterations
   termT: terminating temperature
   */
double SA_Floorplan(FPlan &fp, int k, int local/*=0*/, double term_T/*=0.1*/, int maxIte/*=-1*/ )
{

    // 2005-09-07 (donnie) for soft-resizing in "perturb()" 
    fp.createResizableList();
    
  int MT,uphill,reject;
  double pre_cost,best,cost;
  float d_cost,reject_rate=0;
  
  int N = k * fp.size();
  //float P=0.9;
  float T,actual_T=1;
  double avg=init_avg;
  //float conv_rate = 0.98;
  double time; 
  double time_start;

  //double estimate_avg = 0.08 / avg_ratio;
  //cout << "Estimate Average Delta Cost = " << estimate_avg << endl;
/*
  if(local==0)
    avg = estimate_avg;
*/    

  avg = Random_Floorplan( fp,N);
  avg_ratio = avg;
  //avg = 0.05;
  //avg = 2e-6;
  cout << "Average delta cost = " << avg << endl;
  //estimate_avg = avg;  
  
  time = time_start = seconds();


  T = avg / log(1/P);
  actual_T = T;

  cout << "Intial T = " << T << endl;
  // get inital solution
  fp.packing();
  fp.keep_sol();  
  fp.keep_best();
  pre_cost =  best = fp.getCost();
  
  int good_num=0,bad_num=0;
  double total_cost=0;
  int count=0;
  float p_avg = 0;
  float dcost_avg = 0;
  int bad_count=0;
  int good_count = 0;  
  int itcount = 0;
  //ofstream of("btree_debug");
  //ofstream of2("btree_debug_2");
  //ofstream of3("btree_debug_3");

  //cout << setiosflags( ios::fixed );
  //of2 << setiosflags( ios::fixed );
  //of3 << setiosflags( ios::fixed );

  bool first_feasible = false;

  //double minWasteArea = INT_MAX;    // donnie
  //double minDisplacement = INT_MAX; // donnie

  double bestArea;
  double bestDS;
  double bestWire;
  bool   bestFit;
  double bestDisplacement;
  double bestWaste, bestWidth, bestHeight;
  int    bestCounter=0;
  
  //bestCounter++;
  bestArea = fp.getArea();
  bestDS   = fp.getDeadSpace();
  bestWire = fp.getWireLength();
  bestFit  = fp.isFit();
  bestDisplacement = fp.getTotalModuleDisplacement();
  bestWaste        = fp.getWasteArea();
  bestWidth        = fp.getWidth();
  bestHeight       = fp.getHeight();

  do
  {
      count++;

      total_cost = 0;
      MT = uphill = reject = 0;
      bestCounter = 0;

      printf( "Ite %3d  T= %6.3f", count, T );
      //cout << " T= " << T << endl;

      vector<double> chain; 
      good_count = 0;
      p_avg = 0;
      dcost_avg = 0;

      for(; uphill < N && MT < 2*N; MT++)
      {
	  fp.perturb();
	  fp.packing();
	  
	  /*if( minWasteArea > fp.getWasteArea() )
	  {
	      minWasteArea = fp.getWasteArea();
	      //fp.outputGNUplot( "temp" );
	      cout << "waste area= " << fp.getWasteArea() << endl;
	  }*/
	  
	  
	  cost = fp.getCost(); 
	  d_cost = cost - pre_cost;
	  float p = 1/(exp(d_cost/T));

	  p = p>1? 1:p;

	  itcount++;
	  p_avg+=p;  // observ
	  dcost_avg += fabs(d_cost); // observ

	  chain.push_back(cost);

	  if( d_cost <= 0 || rand_01() < p )
	  {

	      fp.keep_sol();
	      pre_cost = cost;

	      if(d_cost > 0)
	      {       
		  uphill++, bad_num++;
		  //of << d_cost << ": " << p << endl;
	      }
	      else if(d_cost < 0)  
	      {
		   good_num++;
	      }

	      // keep best solution/count
	      if(cost < best)
	      {
		  bool feasible = fp.isFit();

		  //if( first_feasible == false)
		  {
		      fp.keep_best();
		      best = cost;
		  }
		  //else
		  //{
		  //    if( feasible )
		  //    {
			//  fp.keep_best();
			//  best = cost;
		      //}
		  //}

		  if( feasible )
		      first_feasible = true;



		  /*printf("   ==>  C= %f, A= %.2f (%.1f%%), W= %.0f, ar= %.2f, f= %d, dis= %.0f, waste= %.0f, time= %.1f\n", 
			  best, 
			  fp.getArea()*1e-6, 
			  fp.getDeadSpace(),
			  fp.getWireLength()*1e-3,
			  fp.getAR(), fp.isFit(),
			  displacement,
			  fp.getWasteArea(),
			  seconds()-time_start );*/

		  bestCounter++;
		  bestArea = fp.getArea();
		  bestDS   = fp.getDeadSpace();
		  bestWire = fp.getWireLength();
		  bestFit  = fp.isFit();
		  bestDisplacement = fp.getTotalModuleDisplacement();
		  bestWaste        = fp.getWasteArea();
		  bestWidth        = fp.getWidth();
		  bestHeight       = fp.getHeight();
		  
		  // 2005-10-02 (donnie)
		  // if a fixed block is outside of the chip region,
		  // fp.getArea() may smaller than fp.getTotalArea()
		  //assert(fp.getArea() >= fp.getTotalArea());	
		  time = seconds();
		  good_count++;
		  //fp.outDraw( "sa_best.draw" );
		  //fp.outputGNUplot( "sa_best" );
		  //of2 << time-time_start << "\t" << fp.getDeadSpace() << endl;

	      }
	  }
	  else
	  {
	      reject++;
	      fp.recover();
	  }
      }
      if( good_count == 0 /*&& reject_rate > 0.95*/ )
	  bad_count++;
      else 
	  bad_count = 0;

      //p_avg/= MT;
      //dcost_avg /=MT;

      p_avg/= itcount;
      dcost_avg /=itcount;

      //   cout << T << endl;
      //double sv = var(chain);

      //cout << "Average accept rate " << p_avg;
      //cout << " Average dcost:sv " << dcost_avg/sv << " " << dcost_avg << " " << sv << endl;   

      // record T before cooling    
      reject_rate = float(reject)/MT;

      //float r_t = exp(-T/sv); 
      //double r_t = 50*T*T*T/sv;

      //T = actual_T/((float)count/sv);             

      //T = r_t * T;
/*
      if( count < local )
      {
	  T =(actual_T/100)/((double)count/dcost_avg);  
      }
      else
      {
	  T =actual_T/((double)count/dcost_avg);  
      }
*/
      T *= 0.9;
      //printf("  Intial_T= %.2f, r= %f, reject= %.2f, next_T = %f\n\n", actual_T, r_t, reject_rate,T);
      //printf("  r= %f, reject= %.2f\n\n", r_t, reject_rate );
      /*printf( " rej= %.3f  Cost=%.3f A=%2g  DS=%.0f%%  Wire=%5g  Disp=%5g Waste=%5g Good=%d  %d \n", 
	      reject_rate, 
	      best, 
	      bestArea*1e-6, 
	      bestDS, 
	      bestWire, 
	      bestDisplacement, 
	      bestWaste, 
	      bestCounter, 
	      bestFit );*/
     
      printf( " rej= %.3f C= %.3f d=%7.0f A=%9.0f w=%8.0f W/H=%4.0f %4.0f Good= %d %d \n",
	      reject_rate,
	      best,
	      bestDisplacement,
	      bestWaste,
	      bestWire,
	      bestWidth,
	      bestHeight,
	      bestCounter,
	      bestFit );
      
      // After apply local-search, start to use normal SA
      /*
	 if(count == local){
	 T = estimate_avg/ log(P);
	 T *= pow(0.9,local);		// smothing the annealing schedule
	 actual_T = exp(estimate_avg/T);
	 }
	 */


      // stop criteria
      if( reject_rate > 0.98 && bad_count > 10 )
	  break;
      if( reject_rate > 0.95 && bad_count > 40 )
	  break;
      if( maxIte > 0 && count > maxIte )
	  break;
      
  //}while( ( reject_rate<conv_rate && T>1e-4 ) /*|| bad_count<=3*/ );
  }while(true);
    
  //if(reject_rate >= conv_rate)
  //  cout << "\n  Convergent!\n";
  //else if(T <= 1e-4)
  //  cout << "\n Cooling Enough!\n";
  //else
  //  cout << "\n Bad count is "<<bad_count<< " T= "<< T;
  printf("\n good_num = %d, bad_num = %d\n\n", good_num, bad_num);

  fp.recover_best();
  fp.packing();
  return time; 
}

double Random_Floorplan(FPlan &fp,int times)
{
    printf( "Random_Floorplan()\n" );
    int N =times,t=0;
    double total_cost=0,pre_cost,cost,best;

    //printf( "packing()\n" );
    fp.packing();
    //printf( "getCost()\n" );
    pre_cost = best = fp.getCost();
    //printf( "keep_best()\n" );
    fp.keep_best();

    int iteCount = 0;
    do
    {
	for(int i=0; i < N;i++)
	{
	    //printf( "%d ", i );
	    fp.perturb();
	    fp.packing();

	    cost = fp.getCost();
	    if(cost-pre_cost > 0)
	    {
		t++;
		total_cost += cost-pre_cost;
		pre_cost = cost;
	    }

	    if(cost < best)
	    {
		//fp.keep_best();
		best = cost;
		cout << "==> Cost=" << best << endl;
	    }
	}

	pre_cost = best = fp.getCost();
	printf( "pre_cost= %g\n", pre_cost );
	flush( cout );

	
	iteCount++;
	if( iteCount > 10 )
	    break;
    }while(total_cost==0);

    fp.recover_best();
    fp.packing();

    total_cost /= t;
    return total_cost;
}

}
