// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include <iostream>
#include <fstream>
#include "math.h"
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

// Linear Algebra Library
#include <Eigen/Core>
#include <Eigen/Dense>
#include <thread>


// Timer
#include <chrono>
#include <OpenGL/OpenGL.h>

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
Eigen::MatrixXf V(2,3);
Eigen::MatrixXf V_C(2,3);
Eigen::MatrixXf temp2(2,3);
Eigen::Matrix4f view(4,4);
Eigen::MatrixXf animatematrix(3,6);
using namespace std;
int tri_count=0;
int mode=-1;
int tri_num=-1;
double globalx=0;
double globaly=0;
double arr[6];
int tri_rotate=-1;
int ver_color=-1;
int tri_animate=-1;
int counter = -1;

double arrz[9];

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        V.col(0) << p_world[0], p_world[1],1;

    // Upload the change to the GPU
    VBO.update(V);
}

void mouse_button_make_triangle(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);


    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        Eigen::MatrixXf temp(V.rows(),V.cols());
        temp=V;
        //cout<<"Temp is "<<temp<<endl;
        int size = V.cols()+1;
        V.resize(V.rows(),size);
        for(int i=0;i<temp.cols();i++)
        {
            V.col(i) = temp.col(i);
        }
        //cout<<"V aftr copying is"<<V<<endl;
        //cout<<"Adding to V values"<<xworld<< yworld<< 1.0<<endl;
        V.col(size - 1) << p_world[0], p_world[1], 1.0;

        Eigen::MatrixXf tempc(V_C.rows(),V_C.cols());
        tempc=V_C;
//        cout<<"Tempc is "<<tempc<<endl;
        int sizec = V_C.cols()+1;
        V_C.resize(V_C.rows(),sizec);
        for(int i=0;i<tempc.cols();i++)
        {
            V_C.col(i) = tempc.col(i);
        }
        //cout<<"V aftr copying is"<<V<<endl;
        //cout<<"Adding to V values"<<xworld<< yworld<< 1.0<<endl;
        V_C.col(sizec - 1) << 1,0,0;


        //cout<<"V is --> "<<V<<endl;
        tri_count++;
        if(tri_count==3)
        {

            tri_count=0;
            VBO_C.update(V_C);
        }
        VBO.update(V);




    }



}
float area(float x1, float y1, float x2, float y2, float x3, float y3)
{
    return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
}

/* A function to check whether point P(x, y) lies inside the triangle formed
   by A(x1, y1), B(x2, y2) and C(x3, y3) */
bool isInside(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y)
{   cout<<"INSIDE is inside X,Y"<<x<<","<<y<<endl;
    /* Calculate area of triangle ABC */
    float A = area (x1, y1, x2, y2, x3, y3);

    /* Calculate area of triangle PBC */
    float A1 = area (x, y, x2, y2, x3, y3);

    /* Calculate area of triangle PAC */
    float A2 = area (x1, y1, x, y, x3, y3);

    /* Calculate area of triangle PAB */
    float A3 = area (x1, y1, x2, y2, x, y);

    /* Check if sum of A1, A2 and A3 is same as A */
    return (A == A1 + A2 + A3);
}

bool pointInTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y)
{
    float denominator = ((y2 -y3)*(x1 - x3) + (x3 -x2) * (y1 -y3));
    float alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3))/denominator;
    float beta = ((y3 - y1)* (x - x3) + (x1 - x3)*(y - y3))/denominator;
    float gamma = 1 - alpha - beta;

    return (0 <= alpha && alpha <= 1 && 0 <= beta && beta <= 1 && 0 <= gamma && gamma <= 1);

}

int inside_triangle(float xworld,float yworld)
{
    for(int i=3;i<V.cols();i+=3)
    {
        if(pointInTriangle(V(0,i),V(1,i),V(0,i+1),V(1,i+1),V(0,i+2),V(1,i+2),xworld,yworld) ||
           isInside(V(0,i),V(1,i),V(0,i+1),V(1,i+1),V(0,i+2),V(1,i+2),xworld,yworld))
        {
            arr[0]=V(0,i);
            arr[1]=V(1,i);
            arr[2]=V(0,i+1);
            arr[3]=V(1,i+1);
            arr[4]=V(0,i+2);
            arr[5]=V(1,i+2);
            return i;
        }
    }
    return -1;
}
float distance(float dX0, float dY0, float dX1, float dY1)
{
    return sqrt(abs((dX1 - dX0)*(dX1 - dX0) + (dY1 - dY0)*(dY1 - dY0)));
}
int nearest_vertex(float xworld,float yworld)
{
    int vertex=0;
    float dist=std::numeric_limits<float>::max();
    for(int i=3;i<V.cols();i++)
    {   float tempd = distance(xworld,yworld,V(0,i),V(1,i));
        if(tempd<dist)
        {
            vertex=i;
            dist=tempd;
        }
    }
    cout<<"Nearest vertex is "<<vertex<<endl;
    return vertex;

}
void mouse_button_move_triangle(GLFWwindow* window, int button, int action, int mods) {
    cout << "Calling PRESSED----" <<view <<endl;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);



    // Convert screen position to world coordinates
    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    if (inside_triangle((float)p_world[0],(float) p_world[1]) > 0) {
        int start = inside_triangle(p_world[0], p_world[1]);
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

            mode = 1;
            tri_rotate=start;
            tri_num = start;
            globalx = p_world[0];
            globaly = p_world[1];

            arrz[0]=V_C(0,start);
            arrz[1]=V_C(1,start);
            arrz[2]=V_C(2,start);
            arrz[3]=V_C(0,start+1);
            arrz[4]=V_C(1,start+1);
            arrz[5]=V_C(2,start+1);
            arrz[6]=V_C(0,start+2);
            arrz[7]=V_C(1,start+2);
            arrz[8]=V_C(2,start+2);

            V_C(0,start)=0;
            V_C(1,start)=0;
            V_C(2,start)=1;
            V_C(0,start+1)=0;
            V_C(1,start+1)=0;
            V_C(2,start+1)=1;
            V_C(0,start+2)=0;
            V_C(1,start+2)=0;
            V_C(2,start+2)=1;
            VBO_C.update(V_C);

        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            cout << "Calling Released" <<view<< endl;
            mode = 2;
            globalx = 0;
            globaly = 0;
            tri_num = -1;
            V_C(0,start)=arrz[0];
            V_C(1,start)=arrz[1];
            V_C(2,start)=arrz[2];
            V_C(0,start+1)=arrz[3];
            V_C(1,start+1)=arrz[4];
            V_C(2,start+1)=arrz[5];
            V_C(0,start+2)=arrz[6];
            V_C(1,start+2)=arrz[7];
            V_C(2,start+2)=arrz[8];
            VBO_C.update(V_C);
        }

    } else
    {
        tri_count = -1;
    }
}
void mouse_button_delete_triangle(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);



    // Convert screen position to world coordinates
    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    if (inside_triangle(p_world[0], p_world[1]) > 0) {
        int start = inside_triangle(p_world[0], p_world[1]);
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            int x = V.cols();
            Eigen::MatrixXf tempz(V.rows(), x - 3);
            for (int i = 0; i < start; i++) {
                tempz.col(i) = V.col(i);
            }
            for (int i = start; (i < x) && (i+3 <V.cols()) ; i++) {
                tempz.col(i) = V.col(i + 3);
            }
            V.resize(V.rows(), x - 3);
            V = tempz;
            VBO.update(V);


            x = V_C.cols();
            tempz.resize(V_C.rows(), x - 3);
            for (int i = 0; i < start; i++) {
                tempz.col(i) = V_C.col(i);
            }
            for (int i = start; (i < x) && (i+3 <V_C.cols()) ; i++) {
                tempz.col(i) = V_C.col(i + 3);
            }
            V_C.resize(V_C.rows(), x - 3);
            V_C = tempz;
            VBO_C.update(V_C);





        }


    }
}

void mouse_button_animate_triangle(GLFWwindow* window, int button, int action, int mods) {
    cout << "Calling Animation Selection----" <<view <<endl;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);



    // Convert screen position to world coordinates
    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    if (inside_triangle((float)p_world[0],(float) p_world[1]) > 0) {
        int start = inside_triangle(p_world[0], p_world[1]);
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

            mode = 9;
            tri_animate=start;


        }

    } else
    {
        tri_animate = -1;
    }
}
//void mouse_button_rotate_triangle(GLFWwindow* window, int button, int action, int mods) {
//    double xpos, ypos;
//    glfwGetCursorPos(window, &xpos, &ypos);
//    // Get the size of the window
//    int width, height;
//    glfwGetWindowSize(window, &width, &height);
//
//
//
//    // Convert screen position to world coordinates
//    double xworld = ((xpos / double(width)) * 2) - 1;
//    double yworld = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw
//    if (inside_triangle(xworld, yworld) > 0) {
//        int start = inside_triangle(xworld, yworld);
//        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
//           tri_rotate=start;
//            cout<<"Setting triangle to"<<tri_rotate<<endl;
//        }
//    }
//    else
//    {
//        tri_rotate=-1;
//    }
//}

void mouse_button_color_triangle(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);



    // Convert screen position to world coordinates
    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;


    ver_color=nearest_vertex(p_world[0],p_world[1]);
}


void change_color(int vertex,float r, float g , float b)
{
    if(mode==4)
    {
        V_C(0,vertex)=r;
        V_C(1,vertex)=g;
        V_C(2,vertex)=b;
    }
    VBO_C.update(V_C);

}

void moveTriangle(GLFWwindow* window)
{
//    cout<<"Starting point--"<<globalx<<","<<globaly<<endl;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    double xworldn = p_world[0]-globalx;
    double yworldn = p_world[1]-globaly;
//            cout<<"Moving by--"<<xworldn<<","<<yworldn<<endl;
    V(0,tri_num)=arr[0]+xworldn;
    V(1,tri_num)=arr[1]+yworldn;
    V(0,tri_num+1)=arr[2]+xworldn;
    V(1,tri_num+1)=arr[3]+yworldn;
    V(0,tri_num+2)=arr[4]+xworldn;
    V(1,tri_num+2)=arr[5]+yworldn;
    VBO.update(V);

}

void drawline(GLFWwindow* window)
{

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    Eigen::Vector4f p_screen2(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical2((p_screen2[0]/width)*2-1,(p_screen2[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world2 = view.inverse()*p_canonical2;

    temp2.resize(V.rows(),V.cols()+1);
    for(int i=0;i<V.cols();i++)
    {
        temp2.col(i) = V.col(i);
    }
    temp2.col(temp2.cols() - 1) << p_world2[0], p_world2[1], 1.0;
    VBO.update(temp2);




}


void rotate_point(float cx,float cy,float angle,float &px,float &py)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    px -= cx;
    py -= cy;

    // rotate point
    float xnew = px * c - py * s;
    float ynew = px * s + py * c;

    // translate point back:
    px = xnew + cx;
    py = ynew + cy;

}

void scale_point(float cx,float cy,float scale,float &px,float &py)
{

    px = cx + (px-cx)*scale;
    py = cy + (py-cy)*scale;

}


void rotate(GLFWwindow* window, float angle)
{
    if(tri_rotate>=0) {
        float centerX = (V(0, tri_rotate) + V(0, tri_rotate + 1) + V(0, tri_rotate + 2)) / 3;
        float centerY = (V(1, tri_rotate) + V(1, tri_rotate + 1) + V(1, tri_rotate + 2)) / 3;
        cout<<"X,Y before-->"<<V(0, tri_rotate)<<","<<V(1, tri_rotate)<<endl;
        rotate_point(centerX,centerY,angle,V(0, tri_rotate),V(1, tri_rotate));
        rotate_point(centerX,centerY,angle,V(0, tri_rotate+1),V(1, tri_rotate+1));
        rotate_point(centerX,centerY,angle,V(0, tri_rotate+2),V(1, tri_rotate+2));
        cout<<"X,Y after-->"<<V(0, tri_rotate)<<","<<V(1, tri_rotate)<<endl;
        VBO.update(V);

    }

}
void scale(GLFWwindow* window, float scale)
{
    if(tri_rotate>=0) {
        float centerX = (V(0, tri_rotate) + V(0, tri_rotate + 1) + V(0, tri_rotate + 2)) / 3;
        float centerY = (V(1, tri_rotate) + V(1, tri_rotate + 1) + V(1, tri_rotate + 2)) / 3;
        cout<<"Scale X,Y before-->"<<V(0, tri_rotate)<<","<<V(1, tri_rotate)<<endl;
        scale_point(centerX,centerY,scale,V(0, tri_rotate),V(1, tri_rotate));
        scale_point(centerX,centerY,scale,V(0, tri_rotate+1),V(1, tri_rotate+1));
        scale_point(centerX,centerY,scale,V(0, tri_rotate+2),V(1, tri_rotate+2));
        cout<<"Scale X,Y after-->"<<V(0, tri_rotate)<<","<<V(1, tri_rotate)<<endl;
        VBO.update(V);

    }

}

void zoomout()
{
    view << view(0,0)*0.8,0,0,view(0,3),
            0,view(1,1)*0.8,0,view(1,3),
            0,0,view(2,2),0,
            0,0,0,view(3,3);

}

void zoomin()
{
    cout<<"Calling zoomin--> View before -->"<<view<<endl;
    view << view(0,0)*1.2,0,0,view(0,3),
            0,view(1,1)*1.2,0,view(1,3),
            0,0,view(2,2),0,
            0,0,0,view(3,3);

    cout<<"View After -->"<<view<<endl;

}


void pan(GLFWwindow* window,char key)
{


    if(key=='d')
    {
        view << view(0,0),0,0,view(0,3)+0.2,
                0,view(1,1),0,view(1,3),
                0,0,view(2,2),0,
                0,0,0,view(3,3);
    }
    if(key=='a')
    {
        view << view(0,0),0,0,view(0,3)-0.2,
                0,view(1,1),0,view(1,3),
                0,0,view(2,2),0,
                0,0,0,view(3,3);
    }
    if(key=='w')
    {
        view << view(0,0),0,0,view(0,3),
                0,view(1,1),0,view(1,3)+0.2,
                0,0,view(2,2),0,
                0,0,0,view(3,3);
    }
    if(key=='s')
    {
        view << view(0,0),0,0,view(0,3),
                0,view(1,1),0,view(1,3)-0.2,
                0,0,view(2,2),0,
                0,0,0,view(3,3);
    }
}

void store_before()
{

        if(tri_animate>0)
        {
            animatematrix.col(0)=V.col(tri_animate);
            animatematrix.col(1)=V.col(tri_animate+1);
            animatematrix.col(2)=V.col(tri_animate+2);

        }

    cout<<"Stored before"<<animatematrix<<endl;

}

void store_after()
{

        if(tri_animate>0)
        {
            animatematrix.col(3)=V.col(tri_animate);
            animatematrix.col(4)=V.col(tri_animate+1);
            animatematrix.col(5)=V.col(tri_animate+2);
        }
    cout<<"Stored After"<<animatematrix<<endl;
}

void animateposition()
{
    if(mode==9)
    {
        if(tri_animate>0)
        {
//            float stepsx0=(animatematrix(0,3)-animatematrix(0,0))/10;
//            float stepsy0=(animatematrix(1,3)-animatematrix(1,0))/10;
//            float stepsz0=(animatematrix(2,3)-animatematrix(2,0))/10;
//            float stepsx1=(animatematrix(0,4)-animatematrix(0,1))/10;
//            float stepsy1=(animatematrix(1,4)-animatematrix(1,1))/10;
//            float stepsz1=(animatematrix(2,4)-animatematrix(2,1))/10;
//            float stepsx2=(animatematrix(0,5)-animatematrix(0,2))/10;
//            float stepsy2=(animatematrix(1,5)-animatematrix(1,2))/10;
//            float stepsz2=(animatematrix(2,5)-animatematrix(2,2))/10;
//            for(float i = 0;i<10;i++)
//            {   cout<<"Running animation"<<endl;
//                V(0,tri_animate)-=stepsx0;
//                V(1,tri_animate)-=stepsy0;
//                V(2,tri_animate)-=stepsz0;
//                V(0,tri_animate+1)-=stepsx1;
//                V(1,tri_animate+1)-=stepsy1;
//                V(2,tri_animate+1)-=stepsz1;
//                V(0,tri_animate+2)-=stepsx2;
//                V(1,tri_animate+2)-=stepsy2;
//                V(2,tri_animate+2)-=stepsz2;
//
//                std::this_thread::sleep_for(std::chrono::seconds(5));
//
//                VBO.update(V);
//            }
            counter=1;
        }
    }
}


void writetosvg(GLFWwindow* window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ofstream myfile;
    myfile.open ("/Users/Rachit/Desktop/example.svg");
    myfile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    myfile << "<svg width=\""<<width<<"\" height=\""<<height<<"\" version=\"1.200000\" xmlns=\"http://www.w3.org/2000/svg\""
            " xmlns:xlink=\"http://www.w3.org/1999/xlink\"  >\n";

    for(int i=3;i<V.cols();i=i+3)
    {
        Eigen::Vector4f a(V(0,i),V(1,i),0,1);
        Eigen::Vector4f b(V(0,i+1),V(1,i+1),0,1);
        Eigen::Vector4f c(V(0,i+2),V(1,i+2),0,1);
        a= view * a;
        b= view * b;
        c= view * c;

        float ax,ay,bx,by,cx,cy;
        ax=((( a[0] + 1 ) / 2.0) *
            width );
        ay=((( 1 - a[1] ) / 2.0) *
            height );
        bx=((( b[0] + 1 ) / 2.0) *
            width );
        by=((( 1 - b[1] ) / 2.0) *
            height ) ;
        cx=((( c[0] + 1 ) / 2.0) *
            width );
        cy=((( 1 - c[1] ) / 2.0) *
            height );


        myfile << " <defs>\n"
                "      <linearGradient id=\"fadeA-1"<<i<<"\" gradientUnits=\"objectBoundingBox\" x1=\"0.5\" y1=\"0\" x2=\"1\" y2=\"1\">\n"
                "        <stop offset=\"0%\" stop-color=\"rgb("<<roundf((float)V_C(0,i)*255)<<","<<roundf((float)V_C(1,i)*255)<<","<<roundf((float)V_C(2,i)*255)<<")\"/>\n"
                "        <stop offset=\"100%\" stop-color=\"rgb("<<roundf((float)V_C(0,i+2)*255)<<","<<roundf((float)V_C(1,i+2)*255)<<","<<roundf((float)V_C(2,i+2)*255)<<")\"/>\n"
                "      </linearGradient>\n"
                "      <linearGradient id=\"fadeB-1"<<i<<"\" gradientUnits=\"objectBoundingBox\" x1=\"0\" y1=\"1\" x2=\"0.75\" y2=\"0.5\">\n"
                "        <stop offset=\"0%\" stop-color=\"rgb("<<roundf((float)V_C(0,i+1)*255)<<","<<roundf((float)V_C(1,i+1)*255)<<","<<roundf((float)V_C(2,i+1)*255)<<")\"/>\n"
                "        <stop offset=\"100%\" stop-color=\"rgb("<<roundf((float)V_C(0,i+1)*255)<<","<<roundf((float)V_C(1,i+1)*255)<<","<<roundf((float)V_C(2,i+1)*255)<<")\" stop-opacity=\"0\" />\n"
                "      </linearGradient>\n"
                "      </defs>\n"
                "      <path id=\"pathA-1"<<i<<"\" d=\"M "<<ax<<","<<ay<<" L "<<bx<<","<<by<<" "<<cx<<","<<cy<<" Z\" fill=\"url(#fadeA-1"<<i<<")\"/>\n"
                "      <path id=\"pathB-1"<<i<<"\" d=\"M "<<ax<<","<<ay<<" L "<<bx<<","<<by<<" "<<cx<<","<<cy<<" Z\" fill=\"url(#fadeB-1"<<i<<")\"/>\n";
    }


    myfile <<"</svg>\n";
    myfile.close();
    cout<<"Done writing";

}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    if(action==GLFW_PRESS)
        switch (key)
        {
            case  GLFW_KEY_1:
                change_color(ver_color,1.0f,0.0f,0.0f);
                break;
            case GLFW_KEY_2:
                change_color(ver_color,0.0f,1.0f,0.0f);
                break;
            case  GLFW_KEY_3:
                change_color(ver_color,0.0f,0.0f,1.0f);
                break;
            case  GLFW_KEY_4:
                change_color(ver_color,1.0f,1.0f,1.0f);
                break;
            case  GLFW_KEY_5:
                change_color(ver_color,0.0f,0.0f,0.0f);
                break;
            case  GLFW_KEY_6:
                change_color(ver_color,0.5f,0.5f,0.0f);
                break;
            case  GLFW_KEY_7:
                change_color(ver_color,0.0f,0.5f,0.5f);
                break;
            case  GLFW_KEY_8:
                change_color(ver_color,0.5f,0.0f,0.5f);
                break;
            case  GLFW_KEY_9:
                change_color(ver_color,0.25f,0.5f,0.25f);
                break;
            case GLFW_KEY_I:
                mode=0;
                tri_count=0;
                glfwSetMouseButtonCallback(window, mouse_button_make_triangle);
                break;
            case GLFW_KEY_O:
                mode=1;
                glfwSetMouseButtonCallback(window, mouse_button_move_triangle);
                break;
            case GLFW_KEY_P:
                mode=3;
                glfwSetMouseButtonCallback(window, mouse_button_delete_triangle);
                break;
            case GLFW_KEY_C:
                mode=4;
                glfwSetMouseButtonCallback(window, mouse_button_color_triangle);
                break;
//        case GLFW_KEY_T:
//            mode = -1;
//            glfwSetMouseButtonCallback(window, mouse_button_rotate_triangle);
//            break;
            case GLFW_KEY_H:
                //positive
                rotate(window,0.174533);
                break;
            case GLFW_KEY_J:
                //negative
                rotate(window,-0.174533);
                break;
            case GLFW_KEY_K:
                //scaleup
                scale(window,1.25);
                break;
            case GLFW_KEY_L:
                //scaledown
                scale(window,0.75);
                break;
            case GLFW_KEY_EQUAL:
                //Zoomin
                zoomin();
                break;
            case GLFW_KEY_MINUS:
                //Zoomout
                zoomout();
                break;
            case GLFW_KEY_W:
                pan(window,'w');
                break;
            case GLFW_KEY_S:
                pan(window,'s');
                break;
            case GLFW_KEY_A:
                pan(window,'a');
                break;
            case GLFW_KEY_D:
                pan(window,'d');
                break;

            case GLFW_KEY_Q:
                mode=9;
                glfwSetMouseButtonCallback(window, mouse_button_animate_triangle);
                break;

            case GLFW_KEY_Z:
                store_before();
                break;
            case GLFW_KEY_X:
                mode=9;
                store_after();
                animateposition();
                break;
            case GLFW_KEY_U:
                writetosvg(window);
                break;
             default:
                break;
        }

    // Upload the change to the GPU
    VBO.update(V);
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(700, 700, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();

//    V.resize(2,3);
//    V << -0.5,  0.5, 0.5, 0.5, 0.5, -0.5;
//
//    VBO.update(V);

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec2 position;"
                    "in vec3 color;"
                    "out vec3 f_color;"
                    "uniform mat4 view;"
                    "void main()"
                    "{"
                    "    gl_Position =  view * vec4(position, 0.0, 1.0) ;"
                    "    f_color = color;"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "in vec3 f_color;"
                    "out vec4 outColor;"
                    "uniform vec3 triangleColor;"
                    "void main()"
                    "{"
                    "    outColor = vec4(f_color, 1.0);"
                    "}";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
//    program.bindVertexAttribArray("position",VBO);

    //custom code
    VBO.init();
    V.resize(3,3);
//    cout<<"Values after resize are "<<V<<endl;
    V  << 0,0,0,0,0,0,1,1,1;


//    cout<<"orignal V is"<<V<<endl;
//V << 0.5,  -0.5, -0.5,-0.5,0.5,0.5, -0.5, -0.5, 0.5,0.5,0.5,-0.5,1,1,1,1,1,1;
//            -0.5,  0.5, 0.5, 0.5, 0.5, -0.5,1;
    VBO.update(V);

    VBO_C.init();
    V_C.resize(3,3);
    V_C<<
       1,  0, 0,
            1,  0, 0,
            1,  0, 0;

    VBO_C.update(V_C);

    view << 1,0,0,0,
            0,1,0,0,
            0,0,1,0,
            0,0,0,1;




    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color",VBO_C);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
//    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Loop until the user closes the window
    int widths, heights;
    glfwGetWindowSize(window, &widths, &heights);
    float aspect_start=float(heights)/float(widths);
    int i=0;
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
//        VAO.bind();
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspect_ratiox = float(height)/float(heights); // corresponds to the necessary width scaling
        float aspect_ratioy = float(width)/float(widths); // corresponds to the necessary width scaling
        float aspect_ratio=float(height)/float(width);
        if(aspect_ratio!=aspect_start) {
            view(0, 0) = view(0, 0) * aspect_ratioy;
            view(1,1) *= aspect_ratiox;
            aspect_start=aspect_ratio;
            widths=width;
            heights=height;
        }
        // Bind your program
        program.bind();

        // Set the uniform value depending on the time difference
//        auto t_now = std::chrono::high_resolution_clock::now();
//        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
//        glUniform3f(program.uniform("triangleColor"), (float)(sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);
//        glUniform3f(program.uniform("triangleColor"), 1.0f, 0.0f, 0.0f);
        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle
//        if(V.cols()%3==0)
        glDrawArrays(GL_TRIANGLES, 0, V.cols());
//        glDrawArrays(GL_TRIANGLES, 3, 3);
        // Swap front and back buffers
        if(tri_count==1 && mode==0) {

            drawline(window);
            glDrawArrays(GL_LINE_LOOP, temp2.cols()-2, 2);
//            cout<<"temp 2 is --> "<<temp2<<endl;
        }
        if(tri_count==2 && mode==0) {

            drawline(window);
            glDrawArrays(GL_LINE_LOOP, temp2.cols()-3, 3);
//            cout<<"temp 2 is --> "<<temp2<<endl;
        }
        if(mode==1 && tri_num > 0)
        {
            moveTriangle(window);
            int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if(state == GLFW_RELEASE)
            {
                cout<<"Calling Released"<<endl;
                mode = 2;
                globalx=0;
                globaly=0;
                tri_num=-1;
                arr[0]=0;
                arr[1]=0;
                arr[2]=0;
                arr[3]=0;
                arr[4]=0;
                arr[5]=0;
            }
        }
        if(mode==9 && counter==1)
        {
            float stepsx0=(animatematrix(0,3)-animatematrix(0,0))/20;
            float stepsy0=(animatematrix(1,3)-animatematrix(1,0))/20;
            float stepsz0=(animatematrix(2,3)-animatematrix(2,0))/20;
            float stepsx1=(animatematrix(0,4)-animatematrix(0,1))/20;
            float stepsy1=(animatematrix(1,4)-animatematrix(1,1))/20;
            float stepsz1=(animatematrix(2,4)-animatematrix(2,1))/20;
            float stepsx2=(animatematrix(0,5)-animatematrix(0,2))/20;
            float stepsy2=(animatematrix(1,5)-animatematrix(1,2))/20;
            float stepsz2=(animatematrix(2,5)-animatematrix(2,2))/20;
            if (i<=19)
            {   cout<<"Running animation"<<endl;
                V(0,tri_animate)-=stepsx0;
                V(1,tri_animate)-=stepsy0;
                V(2,tri_animate)-=stepsz0;
                V(0,tri_animate+1)-=stepsx1;
                V(1,tri_animate+1)-=stepsy1;
                V(2,tri_animate+1)-=stepsz1;
                V(0,tri_animate+2)-=stepsx2;
                V(1,tri_animate+2)-=stepsy2;
                V(2,tri_animate+2)-=stepsz2;

                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                VBO.update(V);
                i++;
            }
            if(i==20) {
                counter = 0;
                i=0;
            }
        }


        //Binding the view matrix
        glUniformMatrix4fv(program.uniform("view"),1,GL_FALSE,view.data());

        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}