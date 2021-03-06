/*
 *	Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012. PARP Research Group.
 *	<http://perception.inf.um.es>
 *	University of Murcia, Spain.
 *
 *	This file is part of the QVision library.
 *
 *	QVision is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as
 *	published by the Free Software Foundation, version 3 of the License.
 *
 *	QVision is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public
 *	License along with QVision. If not, see <http://www.gnu.org/licenses/>.
 */

/// @file LU-test.cpp
/// @brief Test for LU decomposition based functions from the QVision library.
/// @author PARP Research Group. University of Murcia, Spain.

#include <iostream>

#include <QTime>

#include <QVApplication>
#include <QVPropertyContainer>
#include <QVMatrix>
#include <QVVector>

int main(int argc, char *argv[])
{
    // QVApplication object:
    QVApplication app(argc,argv,"Performance test for LU Decomposition methods in QVision",false);

    // Container with command line parameters:
    QVPropertyContainer arg_container(argv[0]);
    arg_container.addProperty<int>("Rows",QVPropertyContainer::inputFlag,100,
                                   "Number of rows for the test matrix",1,100000);
    arg_container.addProperty<int>("Cols",QVPropertyContainer::inputFlag,100,
                                   "Number of columns for the test matrix",1,100000);
    arg_container.addProperty<int>("RHS_size",QVPropertyContainer::inputFlag,0,
                     "........................Number of right hand sides to solve for:\n"
                     "                                    "
                     "If 0, only decompose, if <0 solve and also return decomposition)",-1000,1000);
    arg_container.addProperty<int>("n_tests",QVPropertyContainer::inputFlag,1,
                                   "Number of tests to average execution time",1,100);
    arg_container.addProperty<QString>("method",QVPropertyContainer::inputFlag,"GSL_LU",
                     "..............................LU method (available methods follow):\n"
                     "                                                                              "
                     "GSL_LU | LAPACK_DGETRF");
    arg_container.addProperty<bool>("verbose",QVPropertyContainer::inputFlag,false,
                                   "If true, print involved matrices and vectors");
    arg_container.addProperty<bool>("show_residuals",QVPropertyContainer::inputFlag,false,
                                   "If true, print test residuals");

    // Process command line (and check for help or incorrect input parameters):
    int ret_value = app.processArguments();
    if(ret_value != 1) exit(ret_value);

    // If parameters OK, read possible parameters from command line:
    const int Rows = arg_container.getPropertyValue<int>("Rows");
    const int Cols = arg_container.getPropertyValue<int>("Cols");
    const int RHS_size = arg_container.getPropertyValue<int>("RHS_size");
    const int n_tests = arg_container.getPropertyValue<int>("n_tests");
    const int verbose = arg_container.getPropertyValue<bool>("verbose");
    const int show_residuals = arg_container.getPropertyValue<bool>("show_residuals");
    const QString method = arg_container.getPropertyValue<QString>("method");
    TQVLU_Method lu_method;
    if(method == "GSL_LU")
        lu_method = GSL_LU;
    else if(method == "LAPACK_DGETRF")
        lu_method = LAPACK_DGETRF;
    else {
        std::cout << "Incorrect LU method. Use --help to see available methods.\n";
        exit(-1);
    }

    std::cout << "Using values: Rows=" << Rows  << " Cols=" << Cols << " RHS_size=" << RHS_size
              << " n_tests=" << n_tests << " LU method=" << qPrintable(method) << "\n";

    double total_ms = 0.0;

    for(int i=0;i<n_tests;i++) {

        QVMatrix M = QVMatrix::random(Rows,Cols), P, L, U, X, B, X_sol;
        QVVector x, b, x_sol;

        // We generate RHS_size random solutions, and the corresponding right hand sides (for testing):
        if(qAbs(RHS_size) == 1) {
            x_sol = QVVector::random(Cols);
            b = M * x_sol;
        } else if (qAbs(RHS_size) > 1) {
            X_sol = QVMatrix::random(Cols,qAbs(RHS_size));
            B = M * X_sol;
        }

        QTime t;

        t.start();

        if(RHS_size == 0)
            LUDecomposition(M, P, L, U, lu_method);
        else if(RHS_size == 1)
            SolveByLUDecomposition(M, x, b, lu_method);
        else if(RHS_size > 1)
            SolveByLUDecomposition(M, X, B, lu_method);
        else if (RHS_size == -1)
            SolveByLUDecomposition(M, x, b, P, L, U, lu_method);
        else if (RHS_size < -1)
            SolveByLUDecomposition(M, X, B, P, L, U, lu_method);

        total_ms += t.elapsed();

        if(verbose) {
            if(RHS_size <= 0) {
                std::cout << "*****************************************\n";
                std::cout << "M:" << M << "\n";
                std::cout << "P:" << P << "\n";
                std::cout << "L:" << L << "\n";
                std::cout << "U:" << U << "\n";
                std::cout << "P L U:" << P*L*U << "\n";
                std::cout << "*****************************************\n";
            }
            if(qAbs(RHS_size) == 1) {
                std::cout << "*****************************************\n";
                std::cout << "M:" << M << "\n";
                std::cout << "b:" << b << "\n";
                std::cout << "x:" << x << "\n";
                std::cout << "x_sol:" << x_sol << "\n";
                std::cout << "*****************************************\n";
            }
            if(qAbs(RHS_size) > 1){
                std::cout << "*****************************************\n";
                std::cout << "M:" << M << "\n";
                std::cout << "B:" << B << "\n";
                std::cout << "X:" << X << "\n";
                std::cout << "X_sol:" << X_sol << "\n";
                std::cout << "*****************************************\n";
            }
        }        

        if(show_residuals) {
            if(RHS_size <= 0) {
                std::cout << "LU residual = " << LUDecompositionResidual(M, P, L, U) << "\n";
            }
            if(qAbs(RHS_size) == 1) {
                std::cout << "Test solve residual = " << SolveResidual(M, x_sol, b) << "\n";
                std::cout << "LU solve residual = " << SolveResidual(M, x, b) << "\n";
                std::cout << "x - x_sol residual = " << (x-x_sol).norm2() << "\n";
                std::cout << "x norm = " << x.norm2() << "\n";
                std::cout << "x_sol norm = " << x_sol.norm2() << "\n";
            }
            if(qAbs(RHS_size) > 1) {
                std::cout << "Test solve residual = " << SolveResidual(M, X_sol, B) << "\n";
                std::cout << "LU solve residual = " << SolveResidual(M, X, B) << "\n";
            }
        }
    }

    total_ms /= n_tests;

    if(n_tests==1)
        std::cout << "Total time: " << total_ms << " ms.\n";
    else
        std::cout << "Average total time: " << total_ms << " ms.\n";

    std::cout << "Finished.\n";    
}
