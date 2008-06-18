/*=========================================================================
Program:   Pointset Registration using Gaussian Mixture Model
Module:    $RCSfile: gmmreg_aux.cpp,v $
Language:  C++
Author:    $Author: bjian $
Date:      $Date: 2008/06/05 17:06:23 $
Version:   $Revision: 1.1 $
=========================================================================*/

#include <iostream>
#include <fstream>
#include <vector>

#include <vnl/vnl_vector.h>
#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_determinant.h>

float estimate_scale(vnl_matrix<float> pts)
{
    int m = pts.rows();
    int d = pts.cols();
    double scale = vnl_determinant(pts.transpose()*pts/(m*1.0));
    for (int i=0;i<d;++i){
        scale = sqrt(scale);
    }
    return scale;
}

/*

def compute_index(pos, x):
	d = len(x)
	index = []
	for i in range(d-1,-1,-1):
		index.insert(0,pos/x[i])
		pos = pos % x[i]
	return index

*/

void compute_index(unsigned long pos, vnl_vector<unsigned long>& v, vnl_vector<int>& index)
{
    int d = v.size(); //assert index.size() = d;
    for (int i=d-1;i>=0;--i){
        index[i] =  pos/v[i];
        pos = pos%v[i];
    }
}

/*
def build_iterator(interval):
	d = len(interval)
	x = [(interval[0:i]).prod()  for i in range(0,d)]
	iter = [compute_index(i,x) for i in range(interval.prod())]
	return iter
*/

void build_iterator(std::vector<int>& interval, vnl_matrix<int>& iterator)
{
    int d = interval.size();
    vnl_vector<unsigned long> v;
    v.set_size(d); v[0] = 1;
    for (int i=1;i<d;++i){
        v[i] = v[i-1]*interval[i-1];
    }
    unsigned long volume = v[d-1]*interval[d-1];
    for (unsigned long i=0; i<volume;++i){
        //compute_index(i, v, iterator.get_row(i));
		unsigned long pos = i;
	    for (int j=d-1;j>=0;--j){
	        iterator(i,j) =  pos/v[j];
		    pos = pos%v[j];
		}
    }
}

/*
def sample_grid(min_pos, max_pos, interval):
	iter = create_iterator(interval)
	float_interval = [float(k)-1 for k in interval]
	grid = [min_pos+(numpy.array(i)/float_interval)*(max_pos-min_pos) for i in iter]
	return numpy.array(grid)
*/



void nd_grid(vnl_vector<float>& min_pos, vnl_vector<float>& max_pos, std::vector<int>& interval, vnl_matrix<int>& iterator, vnl_matrix<float>& matrix)
{
	unsigned long volume = iterator.rows();
	int d = iterator.cols();
    for (unsigned long i=0; i<volume; ++i){
        for (int j=0;j<d;++j){
            matrix(i,j) = min_pos[j] + (max_pos[j]-min_pos[j])*iterator(i,j)*1.0/(interval[j]-1);
        }
    }
}



int main(int argc, char* argv[])
{
	if (argc<3)
	{
		std::cerr << "Usage: " << argv[0] << " InputPtsFile CtrlPtsFile" << std::endl;
		return -1;
	}
	std::ifstream infile(argv[1]);
	vnl_matrix<float> input_pts;
	input_pts.read_ascii(infile);

    int n = input_pts.rows();
    int d = input_pts.cols();

	std::cout << n<< " "<< d << "-d points loaded from "<<argv[1]<<std::endl;
	std::cout << "estimated scale: "<<estimate_scale(input_pts)<<std::endl;
    vnl_vector<float> min_row,max_row;
    min_row.set_size(d);
	max_row.set_size(d);
    for (int i=0;i<d;++i){
        min_row[i] = input_pts.get_column(i).min_value();
		max_row[i] = input_pts.get_column(i).max_value();
    }
	std::vector<int> interval;
	for (int i=0;i<d;++i){
		int k;
		std::cout << "intervals at dimension[" << i << "]:";
		std::cin >> k;
		//assert k>0
		interval.push_back(k);
	}
    //int d = interval.size();  //assert
    unsigned long volume=1;
    for (int i=0;i<d;++i){
        volume *= interval[i];
    }
    vnl_matrix<int> iterator;
    iterator.set_size(volume, d);
    build_iterator(interval, iterator);
	vnl_matrix<float> matrix;
	matrix.set_size(volume, d);
	nd_grid(min_row, max_row, interval, iterator, matrix);
    std::ofstream outfile(argv[2],std::ios_base::out);
    matrix.print(outfile);
    std::cout<< volume << " Control points saved to "<<argv[2]<<std::endl;
	return 0;
}


