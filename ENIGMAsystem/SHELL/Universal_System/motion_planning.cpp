/********************************************************************************\
**                                                                              **
**  Copyright (C) 2011 Harijs Gr�nbergs                                         **
**                                                                              **
**  This file is a part of the ENIGMA Development Environment.                  **
**                                                                              **
**                                                                              **
**  ENIGMA is free software: you can redistribute it and/or modify it under the **
**  terms of the GNU General Public License as published by the Free Software   **
**  Foundation, version 3 of the license or any later version.                  **
**                                                                              **
**  This application and its source code is distributed AS-IS, WITHOUT ANY      **
**  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS   **
**  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more       **
**  details.                                                                    **
**                                                                              **
**  You should have recieved a copy of the GNU General Public License along     **
**  with this code. If not, see <http://www.gnu.org/licenses/>                  **
**                                                                              **
**  ENIGMA is an environment designed to create games and other programs with a **
**  high-level, fully compilable language. Developers of ENIGMA or anything     **
**  associated with ENIGMA are in no way responsible for its users or           **
**  applications created by its users, or damages caused by the environment     **
**  or programs made in the environment.                                        **
**                                                                              **
\********************************************************************************/
#include <vector>
#include "../Graphics_Systems/OpenGL/OpenGLHeaders.h" //For drawing straight lines
//#include "../Graphics_Systems/OpenGL/GSfont.h" //For drawing text
//#include "var4.h"
#include "pathstruct.h"
#include "motion_planning_struct.h"
#include "motion_planning.h"
#include <cmath>
#include <map>
using std::multimap;
//#include <iostream>

namespace enigma {
	extern size_t grid_idmax;
	extern unsigned bound_texture;
}

unsigned mp_grid_create(int left,int top,int hcells,int vcells,int cellwidth,int cellheight, double speed_modifier)
{
    enigma::gridstructarray_reallocate();
    new enigma::grid(enigma::grid_idmax, left, top, hcells, vcells, cellwidth, cellheight, 1, speed_modifier);
    return enigma::grid_idmax-1;
}

void mp_grid_destroy(unsigned id)
{
    delete enigma::gridstructarray[id];
}

void mp_grid_clear_all(unsigned id, unsigned cost)
{
    for (vector<enigma::node>::iterator it = enigma::gridstructarray[id]->nodearray.begin(); it!=enigma::gridstructarray[id]->nodearray.end(); ++it)
        (*it).cost = cost;
    enigma::gridstructarray[id]->threshold = cost;
}

void mp_grid_add_rectangle(unsigned id,double x1,double y1,double x2,double y2, unsigned cost)
{
    enigma::grid *grid = enigma::gridstructarray[id];
    unsigned max_cost=0;
    int tx1 = fmax(floor(fmin(x1,x2)/grid->cellwidth),0), tx2 = fmin(ceil((fmin(x1,x2)+fabs(x2-x1))/grid->cellwidth),grid->hcells),
    ty1 = fmax(floor(fmin(y1,y2)/grid->cellheight),0), ty2 = fmin(ceil((fmin(y1,y2)+fabs(y2-y1))/grid->cellheight),grid->vcells);
    for (unsigned i=tx1; i<tx2; i++){
        for (unsigned c=ty1; c<ty2; c++){
            if (grid->nodearray[i*grid->vcells+c].cost>max_cost){max_cost=grid->nodearray[i*grid->vcells+c].cost;}
            grid->nodearray[i*grid->vcells+c].cost = cost;
        }
    }
    if (cost>max_cost){max_cost=cost;}
    if (grid->threshold<max_cost){grid->threshold=max_cost;}
    //std::cout << "mp_grid_add_rectangle(grid," << floor(x1/grid->cellwidth)*grid->cellwidth << "," << floor(y1/grid->cellheight)*grid->cellheight << "," << ceil(x2/grid->cellwidth)*grid->cellwidth << "," << ceil(y2/grid->cellheight)*grid->cellheight<< ");" << std::endl;
}

void mp_grid_reset_threshold(unsigned id)
{
    enigma::grid *grid = enigma::gridstructarray[id];
    unsigned max_cost=0;
    int vc = grid->vcells;
    for (vector<enigma::node>::iterator it = grid->nodearray.begin(); it!=grid->nodearray.end(); ++it)
        if ((*it).cost>max_cost){max_cost=(*it).cost;}
    grid->threshold=max_cost;
}

void mp_grid_clear_rectangle(unsigned id,double x1,double y1,double x2,double y2, unsigned cost)
{
    mp_grid_add_rectangle(id,x1,y1,x2,y2,cost);
}

void mp_grid_add_cell(unsigned id,int h,int v, unsigned cost)
{
    unsigned max_cost=enigma::gridstructarray[id]->nodearray[h*enigma::gridstructarray[id]->vcells+v].cost;
    enigma::gridstructarray[id]->nodearray[h*enigma::gridstructarray[id]->vcells+v].cost = cost;
    if (cost>max_cost){max_cost=cost;}
    if (enigma::gridstructarray[id]->threshold<max_cost){enigma::gridstructarray[id]->threshold=max_cost;}
}

unsigned mp_grid_get_cell(unsigned id,int h,int v)
{
    return enigma::gridstructarray[id]->nodearray[h*enigma::gridstructarray[id]->vcells+v].cost;
}

unsigned mp_grid_get_threshold(unsigned id)
{
    return enigma::gridstructarray[id]->threshold;
}

void mp_grid_set_threshold(unsigned id, unsigned value)
{
    enigma::gridstructarray[id]->threshold = value;
}

double mp_grid_get_speed_modifier(unsigned id)
{
    return enigma::gridstructarray[id]->speed_modifier;
}

void mp_grid_get_speed_modifier(unsigned id, double value)
{
    enigma::gridstructarray[id]->speed_modifier = value;
}

void mp_grid_path(unsigned id,unsigned pathid,double xstart,double ystart,double xgoal,double ygoal,bool allowdiag)
{
    enigma::grid *gr = enigma::gridstructarray[id];
    int vc = enigma::gridstructarray[id]->vcells,
    xs = floor(xstart/gr->cellwidth), ys = floor(ystart/gr->cellheight),
    xg = floor(xgoal/gr->cellwidth), yg = floor(ygoal/gr->cellheight);
    if (xs<0 or xg<0) return;
    if (xs>gr->hcells-1 or xg>gr->hcells-1) return;
    if (ys<0 or yg<0) return;
    if (ys>gr->vcells-1 or yg>gr->vcells-1) return;
    //if (xstart==xgoal && ystart==ygoal) return;

    bool status = true; //status to check if we can reach the destination
    multimap<unsigned,enigma::node*> nodelist = enigma::find_path(id, &gr->nodearray[xs*vc+ys], &gr->nodearray[xg*vc+yg], allowdiag, status);
    enigma::path *path = enigma::pathstructarray[pathid];
    path->pointarray.clear();

    //push the very first point
    enigma::path_point point={xstart,ystart,gr->speed_modifier/double(gr->nodearray[xs*vc+ys].cost)};
    path->pointarray.push_back(point);
    multimap<unsigned,enigma::node*>::reverse_iterator it;
    for (it=nodelist.rbegin() ; it != nodelist.rend(); it++)
    {
            point=(enigma::path_point){((*it).second->x+0.5)*gr->cellwidth,((*it).second->y+0.5)*gr->cellheight,gr->speed_modifier/double((*it).second->cost)};
            path->pointarray.push_back(point);
    }

    //push the very last point if we can reach the destination
    if (status == true){
        point=(enigma::path_point){xgoal,ygoal,gr->speed_modifier/double(gr->nodearray[xg*vc+yg].cost)};
        path->pointarray.push_back(point);
    }else if (path->pointarray.size()==1){
        point=(enigma::path_point){path->pointarray.back().x,path->pointarray.back().y,gr->speed_modifier/double(gr->nodearray[xg*vc+yg].cost)};
        path->pointarray.push_back(point);
    }
    enigma::path_recalculate(pathid);
}

#include "var4.h"
//#include "GScolors.h"
#define __GETR(x) (((x & 0x0000FF))/255.0)
#define __GETG(x) (((x & 0x00FF00)>>8)/255.0)
#define __GETB(x) (((x & 0xFF0000)>>16)/255.0)
void draw_text(int x,int y,variant str);
int merge_color(int c1,int c2,double amount);
/*void draw_set_halign(int align);
int draw_get_halign();
void draw_set_valign(int align);
int draw_get_valign();
void draw_set_font(int fnt);
int draw_get_font();*/

void mp_grid_draw_neighbours(unsigned id,int h,int v, int mode)
{
    if(enigma::bound_texture) glBindTexture(GL_TEXTURE_2D,enigma::bound_texture = 0);
    enigma::grid *grid = enigma::gridstructarray[id];
    if (h<0) return;
    if (h>grid->hcells-1) return;
    if (v<0) return;
    if (v>grid->vcells-1) return;
    glPushAttrib(GL_CURRENT_BIT);
    glBegin(GL_QUADS);
    glColor4f(0,0,1,mode==0?0.5:1);
    int vc = enigma::gridstructarray[id]->vcells;
    for (vector<enigma::node*>::iterator it = enigma::gridstructarray[id]->nodearray[h*vc+v].neighbor_nodes.begin(); it!=enigma::gridstructarray[id]->nodearray[h*vc+v].neighbor_nodes.end(); ++it){
        glVertex2f((*it)->x*grid->cellwidth,(*it)->y*grid->cellheight);
        glVertex2f(((*it)->x+1)*grid->cellwidth,(*it)->y*grid->cellheight);
        glVertex2f(((*it)->x+1)*grid->cellwidth,((*it)->y+1)*grid->cellheight);
        glVertex2f((*it)->x*grid->cellwidth,((*it)->y+1)*grid->cellheight);
    }
    glEnd();
    glPopAttrib();
    if (mode==1){
        for (vector<enigma::node*>::iterator it = enigma::gridstructarray[id]->nodearray[h*vc+v].neighbor_nodes.begin(); it!=enigma::gridstructarray[id]->nodearray[h*vc+v].neighbor_nodes.end(); ++it){
            draw_text(((*it)->x+0.5)*grid->cellwidth,((*it)->y+0.5)*grid->cellheight,(*it)->x*grid->vcells+(*it)->y);
        }
    }
}

void mp_grid_draw(unsigned id, int mode, unsigned color_mode)
{
    if(enigma::bound_texture) glBindTexture(GL_TEXTURE_2D,enigma::bound_texture = 0);
    enigma::grid *grid = enigma::gridstructarray[id];
    glPushAttrib(GL_CURRENT_BIT);
    glBegin(GL_QUADS);
    int x=grid->top, y=grid->left;
    for (unsigned i=0; i<grid->hcells; i++){
        for (unsigned c=0; c<grid->vcells; c++){
            if (color_mode==0){
                if (grid->nodearray[i*grid->vcells+c].cost < grid->threshold)
                    glColor4f(0,1,0,1);
                else
                    glColor4f(1,0,0,1);
            }else{
                int col = merge_color(0x008000, 0x0000FF, (float)grid->nodearray[i*grid->vcells+c].cost/(float)color_mode);
                glColor4f(__GETR(col),__GETG(col),__GETB(col),1);
            }

            glVertex2f(x+i*grid->cellwidth,y+c*grid->cellheight);
            glVertex2f(x+(i+1)*grid->cellwidth,y+c*grid->cellheight);
            glVertex2f(x+(i+1)*grid->cellwidth,y+(c+1)*grid->cellheight);
            glVertex2f(x+i*grid->cellwidth,y+(c+1)*grid->cellheight);
        }
    }
    glEnd();
    glPopAttrib();

    if (mode){
        /*int temph, tempv, tempf;
        temph = draw_get_halign(), draw_set_halign(1);*/
        /*tempv = draw_get_valign(), draw_set_valign(1);*/
        /*tempf = draw_get_font(), draw_set_font(0);*/
        if (mode==1){ //draw cost
            for (unsigned i=0; i<grid->hcells; i++){
                for (unsigned c=0; c<grid->vcells; c++){
                    draw_text(x+(i+0.5)*grid->cellwidth,y+(c+0.5)*grid->cellheight,grid->nodearray[i*grid->vcells+c].cost);
                }
            }
        }else{ //draw coords
            for (unsigned i=0; i<grid->hcells; i++){
                for (unsigned c=0; c<grid->vcells; c++){
                    draw_text(x+(i+0.5)*grid->cellwidth,y+(c+0.5)*grid->cellheight,string(variant(grid->nodearray[i*grid->vcells+c].x)) + ":" + string(variant(grid->nodearray[i*grid->vcells+c].y)));
                }
            }
        }
        /*draw_set_halign(temph);
        draw_set_valign(tempv);
        draw_set_font(tempf);*/
    }
}
