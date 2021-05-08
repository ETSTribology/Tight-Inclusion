#include <iostream>

#include <tight_inclusion/igl-Timer.h>

#include <tight_inclusion/inclusion_ccd.hpp>
#include <tight_inclusion/interval_root_finder.hpp>
#include <iomanip>
#include <vector>

namespace inclusion_ccd
{


    static double CCD_LENGTH_TOL = 1e-6;


    std::array<Eigen::Vector3d, 2>
    bbd_4_pts_new(const std::array<Eigen::Vector3d, 4> &pts)
    {
        Eigen::Vector3d min, max;
        min = pts[0];
        max = pts[0];
        for (int i = 1; i < 4; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (min[j] > pts[i][j])
                {
                    min[j] = pts[i][j];
                }
                if (max[j] < pts[i][j])
                {
                    max[j] = pts[i][j];
                }
            }
        }
        std::array<Eigen::Vector3d, 2> rst;
        rst[0] = min;
        rst[1] = max;
        return rst;
    }

    double max_diff(
        const double b1min,
        const double b1max,
        const double b2min,
        const double b2max)
    {
        double r = 0;
        if (r < b1max - b1min)
            r = b1max - b1min;
        if (r < b2max - b2min)
            r = b2max - b2min;
        if (r < fabs(b2min - b1max))
            r = fabs(b2min - b1max);
        if (r < fabs(b1min - b2max))
            r = fabs(b1min - b2max);
        return r;
    }

    // calculate maximum x, y and z diff
    double get_max_axis_diff(
        const std::array<Eigen::Vector3d, 2> &b1,
        const std::array<Eigen::Vector3d, 2> &b2)
    {

        double x = max_diff(b1[0][0], b1[1][0], b2[0][0], b2[1][0]);
        double y = max_diff(b1[0][1], b1[1][1], b2[0][1], b2[1][1]);
        double z = max_diff(b1[0][2], b1[1][2], b2[0][2], b2[1][2]);
        return std::max(std::max(x, y), z);
    }

    double max_linf_dist(const Eigen::Vector3d &p1, const Eigen::Vector3d &p2)
    {
        double r = 0;
        for (int i = 0; i < 3; i++)
        {
            if (r < fabs(p1[i] - p2[i]))
            {
                r = fabs(p1[i] - p2[i]);
            }
        }
        return r;
    }

    double max_linf_4(
        const Eigen::Vector3d &p1,
        const Eigen::Vector3d &p2,
        const Eigen::Vector3d &p3,
        const Eigen::Vector3d &p4,
        const Eigen::Vector3d &p1e,
        const Eigen::Vector3d &p2e,
        const Eigen::Vector3d &p3e,
        const Eigen::Vector3d &p4e)
    {
        double r = 0, temp = 0;
        temp = max_linf_dist(p1e, p1);
        if (r < temp)
            r = temp;
        temp = max_linf_dist(p2e, p2);
        if (r < temp)
            r = temp;
        temp = max_linf_dist(p3e, p3);
        if (r < temp)
            r = temp;
        temp = max_linf_dist(p4e, p4);
        if (r < temp)
            r = temp;
        return r;
    }

    Eigen::Vector3d compute_face_vertex_tolerance_3d_new(
        const Eigen::Vector3d &vs,
        const Eigen::Vector3d &f0s,
        const Eigen::Vector3d &f1s,
        const Eigen::Vector3d &f2s,
        const Eigen::Vector3d &ve,
        const Eigen::Vector3d &f0e,
        const Eigen::Vector3d &f1e,
        const Eigen::Vector3d &f2e,
        const double tolerance)
    {
        Eigen::Vector3d p000 = vs - f0s, p001 = vs - f2s,
                        p011 = vs - (f1s + f2s - f0s), p010 = vs - f1s;
        Eigen::Vector3d p100 = ve - f0e, p101 = ve - f2e,
                        p111 = ve - (f1e + f2e - f0e), p110 = ve - f1e;
        double dl = 0;
        double edge0_length = 0;
        double edge1_length = 0;
        dl = 3 * max_linf_4(p000, p001, p011, p010, p100, p101, p111, p110);
        edge0_length = 3 * max_linf_4(p000, p100, p101, p001, p010, p110, p111, p011);
        edge1_length = 3 * max_linf_4(p000, p100, p110, p010, p001, p101, p111, p011);
        // double diag=max_linf_4(
        // p000,p100,p110,p010,
        // p111,p011,p001,p101);

        // std::array<Eigen::Vector3d,2>
        // t0box=bbd_4_pts_new({{p000,p001,p010,p011}});
        // std::array<Eigen::Vector3d,2>
        // t1box=bbd_4_pts_new({{p100,p101,p110,p111}});
        // std::array<Eigen::Vector3d,2>
        // u0box=bbd_4_pts_new({{p000,p001,p100,p101}});
        // std::array<Eigen::Vector3d,2>
        // u1box=bbd_4_pts_new({{p010,p011,p110,p111}});
        // std::array<Eigen::Vector3d,2>
        // v0box=bbd_4_pts_new({{p000,p100,p010,p110}});
        // std::array<Eigen::Vector3d,2>
        // v1box=bbd_4_pts_new({{p001,p101,p011,p111}});
        // dl=get_max_axis_diff(t0box,t1box);
        // edge0_length=get_max_axis_diff(u0box,u1box);
        // edge1_length=get_max_axis_diff(v0box,v1box);
        //     for(int i=0;i<3;i++){
        //         if(dl<fabs(ve[i]-vs[i]))
        //             dl=fabs(ve[i]-vs[i]);

        //         if(dl<fabs(f0e[i]-f0s[i]))
        //             dl=fabs(f0e[i]-f0s[i]);

        //         if(dl<fabs(f1e[i]-f1s[i]))
        //             dl=fabs(f1e[i]-f1s[i]);

        //         if(dl<fabs(f2e[i]-f2s[i]))
        //             dl=fabs(f2e[i]-f2s[i]);

        //         if(edge0_length<fabs(f1s[i] - f0s[i]))
        //             edge0_length=fabs(f1s[i] - f0s[i]);

        //         if(edge0_length<fabs(f1e[i] - f0e[i]))
        //             edge0_length=fabs(f1e[i] - f0e[i]);

        //         if(edge1_length<fabs(f2s[i]-f0s[i]))
        //             edge1_length=fabs(f2s[i]-f0s[i]);

        //         if(edge1_length<fabs(f2e[i]-f0e[i]))
        //             edge1_length=fabs(f2e[i]-f0e[i]);
        //     }
        //    //double edge_length = std::max(edge0_length, edge1_length);
        //     c00=0;c10=0;c20=0;
        //     c00=dl;c10=edge0_length;c20=edge1_length;
        return Eigen::Vector3d(
            tolerance / dl, tolerance / edge0_length, tolerance / edge1_length);
    }

    Eigen::Vector3d compute_edge_edge_tolerance_new(
        const Eigen::Vector3d &edge0_vertex0_start, // a0s
        const Eigen::Vector3d &edge0_vertex1_start, // a1s
        const Eigen::Vector3d &edge1_vertex0_start, // b0s
        const Eigen::Vector3d &edge1_vertex1_start, // b1s
        const Eigen::Vector3d &edge0_vertex0_end,
        const Eigen::Vector3d &edge0_vertex1_end,
        const Eigen::Vector3d &edge1_vertex0_end,
        const Eigen::Vector3d &edge1_vertex1_end,
        const double tolerance)
    {

        Eigen::Vector3d p000 = edge0_vertex0_start - edge1_vertex0_start,
                        p001 = edge0_vertex0_start - edge1_vertex1_start,
                        p011 = edge0_vertex1_start - edge1_vertex1_start,
                        p010 = edge0_vertex1_start - edge1_vertex0_start;
        Eigen::Vector3d p100 = edge0_vertex0_end - edge1_vertex0_end,
                        p101 = edge0_vertex0_end - edge1_vertex1_end,
                        p111 = edge0_vertex1_end - edge1_vertex1_end,
                        p110 = edge0_vertex1_end - edge1_vertex0_end;
        double dl = 0;
        double edge0_length = 0;
        double edge1_length = 0;
        // {
        //     std::array<Eigen::Vector3d,2>
        //     t0box=bbd_4_pts_new({{p000,p001,p010,p011}});
        // std::array<Eigen::Vector3d,2>
        // t1box=bbd_4_pts_new({{p100,p101,p110,p111}});
        // std::array<Eigen::Vector3d,2>
        // u0box=bbd_4_pts_new({{p000,p001,p100,p101}});
        // std::array<Eigen::Vector3d,2>
        // u1box=bbd_4_pts_new({{p010,p011,p110,p111}});
        // std::array<Eigen::Vector3d,2>
        // v0box=bbd_4_pts_new({{p000,p100,p010,p110}});
        // std::array<Eigen::Vector3d,2>
        // v1box=bbd_4_pts_new({{p001,p101,p011,p111}});
        // dl=get_max_axis_diff(t0box,t1box);
        // edge0_length=get_max_axis_diff(u0box,u1box);
        // edge1_length=get_max_axis_diff(v0box,v1box);
        // }

        dl = 3 * max_linf_4(p000, p001, p011, p010, p100, p101, p111, p110);
        edge0_length = 3 * max_linf_4(p000, p100, p101, p001, p010, p110, p111, p011);
        edge1_length = 3 * max_linf_4(p000, p100, p110, p010, p001, p101, p111, p011);
        // for(int i=0;i<3;i++){
        //     if(fabs(p000[i]-p100[i])>dl)
        //         dl=fabs(p000[i]-p100[i]);
        //     if(fabs(p001[i]-p101[i])>dl)
        //         dl=fabs(p001[i]-p101[i]);
        //     if(fabs(p010[i]-p110[i])>dl)
        //         dl=fabs(p010[i]-p110[i]);
        //     if(fabs(p011[i]-p111[i])>dl)
        //         dl=fabs(p011[i]-p111[i]);
        // }

        //    double dl=0;
        //    double edge0_length=0;
        //    double edge1_length=0;
        //     for(int i=0;i<3;i++){
        //         if(dl<fabs(edge0_vertex0_end[i]-edge0_vertex0_start[i]))
        //             dl=fabs(edge0_vertex0_end[i]-edge0_vertex0_start[i]);

        //         if(dl<fabs(edge0_vertex1_end[i]-edge0_vertex1_start[i]))
        //             dl=fabs(edge0_vertex1_end[i]-edge0_vertex1_start[i]);

        //         if(dl<fabs(edge1_vertex0_end[i]-edge1_vertex0_start[i]))
        //             dl=fabs(edge1_vertex0_end[i]-edge1_vertex0_start[i]);

        //         if(dl<fabs(edge1_vertex1_end[i]-edge1_vertex1_start[i]))
        //             dl=fabs(edge1_vertex1_end[i]-edge1_vertex1_start[i]);

        //         if(edge0_length<fabs(edge0_vertex1_start[i] -
        //         edge0_vertex0_start[i]))
        //             edge0_length=fabs(edge0_vertex1_start[i] -
        //             edge0_vertex0_start[i]);

        //         if(edge0_length<fabs(edge0_vertex1_end[i] -
        //         edge0_vertex0_end[i]))
        //             edge0_length=fabs(edge0_vertex1_end[i] -
        //             edge0_vertex0_end[i]);

        //         if(edge1_length<fabs(edge1_vertex1_start[i] -
        //         edge1_vertex0_start[i]))
        //             edge1_length=fabs(edge1_vertex1_start[i] -
        //             edge1_vertex0_start[i]);

        //         if(edge1_length<fabs(edge1_vertex1_end[i] -
        //         edge1_vertex0_end[i]))
        //             edge1_length=fabs(edge1_vertex1_end[i] -
        //             edge1_vertex0_end[i]);
        // }
        return Eigen::Vector3d(
            tolerance / dl, tolerance / edge0_length, tolerance / edge1_length);
    }

    // This function can give you the answer of continous collision detection with minimum
    // seperation, and the earlist collision time if collision happens.
    // err is the filters calculated using the bounding box of the simulation scene.
    // If you are checking a single query without a scene, please set it as [-1,-1,-1].
    // ms is the minimum seperation. should set: ms < max(abs(x),1), ms < max(abs(y),1), ms < max(abs(z),1) of the QUERY (NOT THE SCENE!).
    // toi is the earlist time of collision if collision happens. If there is no collision, toi will be infinate.
    // tolerance is a user - input solving precision. we suggest to use 1e-6.
    // t_max is the upper bound of the time interval [0,t_max] to be checked. 0<=t_max<=1
    // max_itr is a user-defined value to terminate the algorithm earlier, and return a result under current
    // precision. please set max_itr either a big number like 1e7, or -1 which means it will not be terminated
    // earlier and the precision will be user-defined precision -- tolerance.
    // output_tolerance is the precision under max_itr ( > 0). if max_itr < 0, output_tolerance = tolerance;
    // CCD_TYPE is a switch to choose root-finding methods.
    // 0 is normal ccd,
    // 1 is ccd with input time interval upper bound, using real tolerance, max_itr and horizontal tree,
    bool edgeEdgeCCD_double(
        const Eigen::Vector3d &a0s,
        const Eigen::Vector3d &a1s,
        const Eigen::Vector3d &b0s,
        const Eigen::Vector3d &b1s,
        const Eigen::Vector3d &a0e,
        const Eigen::Vector3d &a1e,
        const Eigen::Vector3d &b0e,
        const Eigen::Vector3d &b1e,
        const std::array<double, 3> &err,
        const double ms,
        double &toi,
        const double tolerance,
        const double t_max,
        const int max_itr,
        double &output_tolerance,
        const int CCD_TYPE)
    {

        Eigen::Vector3d tol = compute_edge_edge_tolerance_new(
            a0s, a1s, b0s, b1s, a0e, a1e, b0e, b1e, tolerance);

        //this should be the error of the whole mesh
        std::array<double, 3> err1;
        if (err[0] < 0)
        { // if error[0]<0, means we need to calculate error here
            std::vector<Eigen::Vector3d> vlist;
            vlist.emplace_back(a0s);
            vlist.emplace_back(a1s);
            vlist.emplace_back(b0s);
            vlist.emplace_back(b1s);

            vlist.emplace_back(a0e);
            vlist.emplace_back(a1e);
            vlist.emplace_back(b0e);
            vlist.emplace_back(b1e);
            bool use_ms = ms > 0;
            err1 = get_numerical_error(vlist, false, use_ms);
        }
        else
        {
            err1 = err;
        }

        //////////////////////////////////////////////////////////


        bool is_impacting;

        // 0 is normal ccd without pre-check,
        // 1 is ccd without pre-check, using real tolerance and horizontal tree,
        // 2 is ccd with pre-check, using real tolerance and  horizontal tree
        // int CCD_TYPE=2;
        if (CCD_TYPE == 0)
        {
            is_impacting = interval_root_finder_double_normalCCD(
                tol, toi, false, err1, ms, a0s, a1s, b0s, b1s, a0e, a1e, b0e, b1e);
        }
        if (CCD_TYPE == 1)
        {
            assert(t_max >= 0 && t_max <= 1);
            is_impacting = interval_root_finder_double_horizontal_tree(
                tol, tolerance, toi, false, err1, ms, a0s, a1s, b0s, b1s, 
                a0e, a1e, b0e, b1e, t_max, max_itr, output_tolerance);
        }


        // Return a conservative time-of-impact
        //    if (is_impacting) {
        //        toi =
        //        double(toi_interval[0].first.first)/pow(2,toi_interval[0].first.second);
        //    }
        // This time of impact is very dangerous for convergence
        assert(!is_impacting || toi >= 0);
#ifdef TIGHT_INCLUSION_NO_ZERO_TOI

        // this modification is for IPC simulation
        if(toi==0){
            // std::cout<<"ee toi == 0, info:\n"<<"tolerance "<<tolerance<<"\noutput_tolerance "<<
            // output_tolerance<<"\nminimum distance "<<ms<<std::endl;
            // std::cout<<"ms > 0? "<<(ms>0)<<std::endl;
            // std::cout<<"t max "<<t_max<<std::endl;

            // we rebuild the time interval
            // since tol is conservative:
            double new_max_time=std::min(tol[0]*10, 0.1);// this is the new time range    
            //if early terminated, use tolerance; otherwise, use smaller tolerance
            // althouth time resolution and tolerance is not the same thing, but decrease
            // tolerance will be helpful
            double new_tolerance=output_tolerance>tolerance?tolerance:0.1*tolerance;
            double new_toi;
            double new_output_tol;
            bool res=edgeEdgeCCD_double(a0s, a1s, b0s, b1s, 
                a0e, a1e, b0e, b1e,
                err,ms,new_toi,new_tolerance,new_max_time,max_itr,new_output_tol,
               CCD_TYPE);

            if(res){
                toi=new_toi;
            }
            else{
                toi=new_max_time;
            }
            return true;
        }

#endif

        //modification end
        return is_impacting;
        return false;
    }
    int LEVEL_NBR=0;
    // This function can give you the answer of continous collision detection with minimum
    // seperation, and the earlist collision time if collision happens.
    // err is the filters calculated using the bounding box of the simulation scene.
    // If you are checking a single query without a scene, please set it as [-1,-1,-1].
    // ms is the minimum seperation. should set: ms < max(abs(x),1), ms < max(abs(y),1), ms < max(abs(z),1) of the QUERY (NOT THE SCENE!).
    // toi is the earlist time of collision if collision happens. If there is no collision, toi will be infinate.
    // tolerance is a user - input solving precision. we suggest to use 1e-6.
    // t_max is the upper bound of the time interval [0,t_max] to be checked. 0<=t_max<=1
    // max_itr is a user-defined value to terminate the algorithm earlier, and return a result under current
    // precision. please set max_itr either a big number like 1e7, or -1 which means it will not be terminated
    // earlier and the precision will be user-defined precision -- tolerance.
    // output_tolerance is the precision under max_itr ( > 0). if max_itr < 0, output_tolerance = tolerance;
    // CCD_TYPE is a switch to choose root-finding methods.
    // 0 is normal ccd,
    // 1 is ccd with input time interval upper bound, using real tolerance, max_itr and horizontal tree,

    bool vertexFaceCCD_double(
        const Eigen::Vector3d &vertex_start,
        const Eigen::Vector3d &face_vertex0_start,
        const Eigen::Vector3d &face_vertex1_start,
        const Eigen::Vector3d &face_vertex2_start,
        const Eigen::Vector3d &vertex_end,
        const Eigen::Vector3d &face_vertex0_end,
        const Eigen::Vector3d &face_vertex1_end,
        const Eigen::Vector3d &face_vertex2_end,
        const std::array<double, 3> &err,
        const double ms,
        double &toi,
        const double tolerance,
        const double t_max,
        const int max_itr,
        double &output_tolerance,
        const int CCD_TYPE)
    {

        Eigen::Vector3d tol = compute_face_vertex_tolerance_3d_new(
            vertex_start, face_vertex0_start, face_vertex1_start,
            face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
            face_vertex2_end, tolerance);

        //////////////////////////////////////////////////////////
        // this is the error of the whole mesh
        std::array<double, 3> err1;
        if (err[0] < 0)
        { // if error[0]<0, means we need to calculate error here
            std::vector<Eigen::Vector3d> vlist;
            vlist.emplace_back(vertex_start);
            vlist.emplace_back(face_vertex0_start);
            vlist.emplace_back(face_vertex1_start);
            vlist.emplace_back(face_vertex2_start);

            vlist.emplace_back(vertex_end);
            vlist.emplace_back(face_vertex0_end);
            vlist.emplace_back(face_vertex1_end);
            vlist.emplace_back(face_vertex2_end);
            bool use_ms = ms > 0;
            err1 = get_numerical_error(vlist, false, use_ms);
        }
        else
        {
            err1 = err;
        }
        //////////////////////////////////////////////////////////

        // Interval3 toi_interval;
        
        bool is_impacting;

        // 0 is normal ccd without pre-check,
        // 1 is ccd without pre-check, using real tolerance and horizontal tree,
        // 2 is ccd with pre-check, using real tolerance and  horizontal tree
        // int CCD_TYPE=2;
        if (CCD_TYPE == 0)
        {
            is_impacting = interval_root_finder_double_normalCCD(
                tol, toi, true, err1, ms, vertex_start, face_vertex0_start, face_vertex1_start,
                face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
                face_vertex2_end);
        }
        if (CCD_TYPE == 1)
        {
            assert(t_max >= 0 && t_max <= 1);
            is_impacting = interval_root_finder_double_horizontal_tree(
                tol, tolerance, toi, true, err1, ms, vertex_start, face_vertex0_start, face_vertex1_start,
                face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
                face_vertex2_end, t_max, max_itr, output_tolerance);
        }

        // Return a conservative time-of-impact
        //    if (is_impacting) {
        //        toi =
        //        double(toi_interval[0].first.first)/pow(2,toi_interval[0].first.second);
        //    }
        // This time of impact is very dangerous for convergence
        // assert(!is_impacting || toi > 0);


        // this modification is for IPC simulation
        LEVEL_NBR++;
#ifdef TIGHT_INCLUSION_NO_ZERO_TOI
        if(toi==0){
            // std::cout<<"vf toi == 0, info:\n"<<"tolerance "<<tolerance<<"\noutput_tolerance "<<
            // output_tolerance<<"\nminimum distance "<<std::setprecision(17)<<ms<<std::endl;
            // std::cout<<"ms > 0? "<<(ms>0)<<std::endl;
            // std::cout<<"t max "<<t_max<<std::endl;
            // std::cout<<"which level "<<LEVEL_NBR<<std::endl;

            // we rebuild the time interval
            // since tol is conservative:
            double new_max_time=std::min(tol[0]*10, 0.1);// this is the new time range    
            //if early terminated, use tolerance; otherwise, use smaller tolerance
            // althouth time resolution and tolerance is not the same thing, but decrease
            // tolerance will be helpful
            double new_tolerance=output_tolerance>tolerance?tolerance:0.1*tolerance;
            double new_toi;
            double new_output_tol;
            bool res=vertexFaceCCD_double(vertex_start, face_vertex0_start, face_vertex1_start,
                face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
                face_vertex2_end,err,ms,new_toi,new_tolerance,new_max_time,max_itr,new_output_tol,
               CCD_TYPE);

            if(res){
                toi=new_toi;
            }
            else{
                toi=new_max_time;
            }
            LEVEL_NBR=0;
            return true;
        }
#endif  
        LEVEL_NBR=0;

        return is_impacting;
        return false;
    }

#ifdef TIGHT_INCLUSION_USE_GMP
    bool edgeEdgeCCD_rational(
        const Eigen::Vector3d &a0s,
        const Eigen::Vector3d &a1s,
        const Eigen::Vector3d &b0s,
        const Eigen::Vector3d &b1s,
        const Eigen::Vector3d &a0e,
        const Eigen::Vector3d &a1e,
        const Eigen::Vector3d &b0e,
        const Eigen::Vector3d &b1e,
        const std::array<double, 3> &err,
        const double ms,
        double &toi)
    {

        Eigen::Vector3d tol = compute_edge_edge_tolerance_new(a0s, a1s, b0s, b1s, a0e, a1e, b0e, b1e, CCD_LENGTH_TOL);

        //////////////////////////////////////////////////////////
        // TODO this should be the error of the whole mesh
        std::vector<Eigen::Vector3d> vlist;
        vlist.emplace_back(a0s);
        vlist.emplace_back(a1s);
        vlist.emplace_back(b0s);
        vlist.emplace_back(b1s);

        vlist.emplace_back(a0e);
        vlist.emplace_back(a1e);
        vlist.emplace_back(b0e);
        vlist.emplace_back(b1e);

        std::array<double, 3> err1;
        bool use_ms = ms > 0;
        err1 = get_numerical_error(vlist, false, use_ms);
        //////////////////////////////////////////////////////////

        std::array<std::pair<Rational, Rational>, 3> toi_interval;

        bool is_impacting = interval_root_finder_Rational(
            tol, toi_interval, false, err1, ms, a0s, a1s, b0s, b1s, a0e, a1e, b0e,
            b1e);
        
        // Return a conservative time-of-impact
        if (is_impacting)
        {
            toi = toi_interval[0].first.to_double();
        }
        // This time of impact is very dangerous for convergence
        // assert(!is_impacting || toi > 0);
        return is_impacting;
        return false;
    }

    bool vertexFaceCCD_rational(
        const Eigen::Vector3d &vertex_start,
        const Eigen::Vector3d &face_vertex0_start,
        const Eigen::Vector3d &face_vertex1_start,
        const Eigen::Vector3d &face_vertex2_start,
        const Eigen::Vector3d &vertex_end,
        const Eigen::Vector3d &face_vertex0_end,
        const Eigen::Vector3d &face_vertex1_end,
        const Eigen::Vector3d &face_vertex2_end,
        const std::array<double, 3> &err,
        const double ms,
        double &toi)
    {

        Eigen::Vector3d tol = compute_face_vertex_tolerance_3d_new(
            vertex_start, face_vertex0_start, face_vertex1_start,
            face_vertex2_start, vertex_end, face_vertex0_end, face_vertex1_end,
            face_vertex2_end, CCD_LENGTH_TOL);
        // std::cout<<"get tolerance successfully"<<std::endl;
        //////////////////////////////////////////////////////////
        // TODO this should be the error of the whole mesh
        std::vector<Eigen::Vector3d> vlist;
        vlist.emplace_back(vertex_start);
        vlist.emplace_back(face_vertex0_start);
        vlist.emplace_back(face_vertex1_start);
        vlist.emplace_back(face_vertex2_start);

        vlist.emplace_back(vertex_end);
        vlist.emplace_back(face_vertex0_end);
        vlist.emplace_back(face_vertex1_end);
        vlist.emplace_back(face_vertex2_end);

        std::array<double, 3> err1;
        bool use_ms = ms > 0;
        err1 = get_numerical_error(vlist, false, use_ms);
        // std::cout<<"get error successfully"<<std::endl;
        //////////////////////////////////////////////////////////

        std::array<std::pair<Rational, Rational>, 3> toi_interval;

        bool is_impacting = interval_root_finder_Rational(
            tol, toi_interval, true, err1, ms, vertex_start, face_vertex0_start,
            face_vertex1_start, face_vertex2_start, vertex_end, face_vertex0_end,
            face_vertex1_end, face_vertex2_end);
        
        // std::cout<<"get result successfully"<<std::endl;
        // Return a conservative time-of-impact
        if (is_impacting)
        {
            toi = toi_interval[0].first.to_double();
        }
        // std::cout<<"get time successfully"<<std::endl;
        // This time of impact is very dangerous for convergence
        // assert(!is_impacting || toi > 0);
        return is_impacting;
        return false;
    }
#endif
    
bool using_rational_method(){
    #ifdef TIGHT_INCLUSION_USE_GMP
    return true;
    #else
    return false;
    #endif
}    
    
} // namespace inclusion_ccd
