#include <QCoreApplication>
#include<opencv/cv.h>
#include<opencv/highgui.h>
#include<complex>
using namespace cv;
using namespace std;

Mat crossProduct(Mat a, Mat b);

Mat homogeneousPoint(double x,double y)
{
    return (Mat_<double>(3,1) << x,y,1);
}

Mat homogeneousLine(Vec4i Line)
{
    Mat PointA = homogeneousPoint((double)Line[0],(double)Line[1]);
    Mat PointB = homogeneousPoint((double)Line[2],(double)Line[3]);
    Mat LineCoordinate = crossProduct(PointA,PointB);
    double length = LineCoordinate.at<double>(0,0) * LineCoordinate.at<double>(0,0) + LineCoordinate.at<double>(1,0) * LineCoordinate.at<double>(1,0) + LineCoordinate.at<double>(2,0) * LineCoordinate.at<double>(2,0);
    length = sqrt(length) / 1000;
    for(uchar i = 0;i < 3;i++)
    {
        LineCoordinate.at<double>(i,0) /= length;
    }
    return LineCoordinate;
}

void MyLine(Mat Img, Mat Line)
{
    int PosX[2] = {0,0};
    int PosY[2] = {0,0};
    bool isOnLine[4]= {false,false,false,false};
    Mat X0 = (Mat_<double>(3,1) << 0,1,0);
    Mat X1 = (Mat_<double>(3,1) << 0,1,1 - Img.rows);
    Mat Y0 = (Mat_<double>(3,1) << 1,0,0);
    Mat Y1 = (Mat_<double>(3,1) << 1,0,1 - Img.cols);
    Mat Insert[4] = Mat_<double>(3,1);
    Insert[0] = crossProduct(Line,X0);
    Insert[1] = crossProduct(Line,X1);
    Insert[2] = crossProduct(Line,Y0);
    Insert[3] = crossProduct(Line,Y1);
    for(uchar i = 0;i < 4;i++)
    {
        if(Insert[i].at<double>(0,0)/Insert[i].at<double>(2,0) >= 0 && Insert[i].at<double>(0,0)/Insert[i].at<double>(2,0) < Img.cols)
        if(Insert[i].at<double>(1,0)/Insert[i].at<double>(2,0) >= 0 && Insert[i].at<double>(1,0)/Insert[i].at<double>(2,0) < Img.rows)
            isOnLine[i] = true;
    }
    uchar j = 0;
    for(uchar i = 0;i < 4;i++)
    {
        if(isOnLine[i])
        {
            PosX[j] = (int)Insert[j].at<double>(0,0)/Insert[j].at<double>(2,0);
            PosY[j] = (int)Insert[j].at<double>(1,0)/Insert[j].at<double>(2,0);
            j++;
        }
    }
    line(Img,Point(PosX[0],PosY[0]),Point(PosX[1],PosY[1]),Scalar(255,0,0), 3, CV_AA);
}
//check whether two lines are colinear
bool isColinear(Mat a,Mat b)
{
    double temp;
    Mat Product = crossProduct(a,b);
    temp = (Product.at<double>(0,0)) * (Product.at<double>(0,0)) + (Product.at<double>(1,0) ) * (Product.at<double>(1,0)) + (Product.at<double>(2,0)) * (Product.at<double>(2,0));
    if(temp < 100000)
        return true;
    else
        return false;
}

Mat MultipleByMatrix(Mat TransMatrix,Mat IVector)
{
    Mat OVector = Mat_<double>(3,1);
    //only accepted when the Matrix's cols number is three
    if (TransMatrix.cols == 3)
    {
        for (int i = 0;i < 3;i++)
        {
            double *pM = TransMatrix.ptr<double>(i);
            double *pOV = OVector.ptr<double>(i);
            for (int j = 0;j < 3;j++)
            {
                pOV[0] += pM[j] * IVector.at<double>(j,0);
            }
        }
    }
    else
    {
        cout << "Invalid Input" << endl;
        return 1;
    }
    OVector.at<double>(0,0) /= OVector.at<double>(2,0);
    OVector.at<double>(1,0) /= OVector.at<double>(2,0);
    OVector.at<double>(2,0) = 1;
    return OVector;
}

Mat MatrixTransform(Mat InputImg,Mat TransMatrix)
{
    Mat OutputImg = Mat::zeros(InputImg.size(), InputImg.type());
    double nl = InputImg.rows;
    double nc = InputImg.cols;
    int channel = InputImg.channels();
    cout << OutputImg.rows << endl;
    cout << InputImg.rows << endl;
    cout << OutputImg.cols << endl;
    cout << InputImg.cols << endl;
    cout << OutputImg.channels() << endl;
    cout << InputImg.channels() << endl;
    for(int j = 0;j < nl;j++)
    {
        uchar *p = InputImg.ptr<uchar>(j);
        for(int i = 0;i < nc;i++)
        {
            Mat Ipos = (Mat_<double>(3,1) << i,j,1);
            Mat Opos = MultipleByMatrix(TransMatrix,Ipos);
            if(Opos.at<double>(0,0) < nc && Opos.at<double>(0,0) > 0 && Opos.at<double>(1,0) < nl && Opos.at<double>(1,0) > 0)
            {
                int ii = (int)Opos.at<double>(0,0);
                int jj = (int)Opos.at<double>(1,0);
                if(OutputImg.at<uchar>(ii,jj) == 0 )
                {
                    switch(channel)
                    {
                    case 1:
                    {
                        OutputImg.at<uchar>(ii,jj) = p[i];
                        break;
                    }
                    case 3:
                    {
                        for(int Cnum = 0;Cnum < channel;Cnum ++)
                        {
                            OutputImg.at<Vec3b>(ii,jj)[Cnum] = p[channel * i + Cnum];
//                    cout << p[i]  << endl;
                        }
                        break;
                    }
                    }
                }
                else
                {
//                  OutputImg.at<uchar>(ii,jj) = (uchar)0.5 * (p[i]+OutputImg.at<uchar>(ii,jj));
                }
            }
        }
    }
    return OutputImg;
}

//compute the prospective transform matrix
Mat MapToAffineMatrix()
{

}

Mat crossProduct(Mat a, Mat b)
{
    Mat I = (Mat_<double>(3,1)<<-1,-1,-1);
    if(a.rows == 3 && a.cols == 1&&b.rows == 3 &&b.cols ==1)//only 3d vector is accepted
    {
        I = (Mat_<double>(3,1)<<a.at<double>(1,0)*b.at<double>(2,0)-a.at<double>(2,0)*b.at<double>(1,0),a.at<double>(2,0)*b.at<double>(0,0)-a.at<double>(0,0)*b.at<double>(2,0),a.at<double>(0,0)*b.at<double>(1,0)-a.at<double>(1,0)*b.at<double>(0,0));
    }
    else
        cout <<"Invalid input"<<endl;
    return I;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    VideoCapture cap(0);
//    if(!cap.isOpened()) return -1;

    Mat src, cosrc,spin;
    Mat dst, cdst;
    Mat TransMatrix = (Mat_<double>(3,3) << 0.5 * sqrt(3),-0.5 * sqrt(3),0,0.5,0.5,0,0,0,1);
    src=imread("/home/cyw/MultiView/MultiView1/src.jpg",0);
    cosrc = imread("/home/cyw/MultiView/MultiView1/src.jpg",1);
    imshow("original", cosrc);
    spin = MatrixTransform(cosrc,TransMatrix);
    cvtColor(src, src, CV_GRAY2BGR);
//    cout << cosrc.type() << endl;
    threshold(src,src,200,255,THRESH_BINARY);

    imshow("spin",spin);
//    Mat aa = (Mat_<int>(3,1)<<1,2,3);
//    Mat bb = (Mat_<int>(3,1)<<1,1,1);
//    cout << crossProduct(aa,bb) <<endl;
//    namedWindow("edges",1);
//    for(;;)
    {
//        cap >> src;
//        Mat kern = (Mat_<char>(3,3) <<
//        0, -.5, 0,
//        -.5, 3, -.5,
//        0, -.5, 0);
//        filter2D(src, src, src.depth(), kern);
        Canny(src, dst, 180, 250, 3);
        cvtColor(dst, cdst, CV_GRAY2BGR);
        vector<Vec4i> lines;
        HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
        Mat Line[lines.size()] = Mat_<double>(3,1);
        cout << lines.size()<<endl;
        for( size_t i = 0; i < lines.size(); i++ )
        {
            Vec4i l = lines[i];
            Line[i] = homogeneousLine(l);
            line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
//            cout << Line[i]<<endl;
        }
        //check whether the lines in the buff is the same one
        for(int i = 0;i < lines.size();i++)
        {
            for(int j = i+1;j < lines.size();j++)
            {
                if(isColinear(Line[i],Line[j]))
                {
                    Line[i].at<double>(0,0) = 0.5 *(Line[i].at<double>(0,0) + Line[j].at<double>(0,0));
                    Line[i].at<double>(1,0) = 0.5 *(Line[i].at<double>(1,0) + Line[j].at<double>(1,0));
                    Line[i].at<double>(2,0) = 0.5 *(Line[i].at<double>(2,0) + Line[j].at<double>(2,0));
                    Line[j].at<double>(0,0) = 0;
                    Line[j].at<double>(1,0) = 0;
                    Line[j].at<double>(2,0) = 1;
                }
            }
        }
        for(int i = 0;i < lines.size();i++)
        {
//            if(Line[i].at<int32_t>(0,0) != 0)
            {
                cout << Line[i] << endl;
                cout << "........................." << endl;
                MyLine(cdst,Line[i]);
            }
        }
        imshow("source", src);
        imshow("detected lines", cdst);
//        if(waitKey(30) >= 0) break;
    }    
//    cvtColor(frame, edges, CV_BGR2GRAY);
//    GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
//    Canny(edges, edges, 0, 30, 3);

    return a.exec();
}

