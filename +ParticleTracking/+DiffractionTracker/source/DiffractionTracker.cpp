#include <extras/cmex/NumericArray.hpp>
#include <extras/async/ProcessorPersistentArgs.hpp>
#include <extras/SessionManager/ObjectManager.h> // Object manager includes
#include <extras/SessionManager/mexDispatch.h>

/// Define DiffractionTracker Class
class diffProc:public extras::async::ProcessorWithPersistentArgs{//extras::async::AsyncProcessor{//
protected:

    std::shared_ptr<extras::cmex::mxObject> ROIstruct;
    std::shared_ptr<

    /// method for Processing Tasks in the task list
    virtual extras::cmex::mxArrayGroup ProcessTask(const std::pair<extras::cmex::mxArrayGroup,std::shared_ptr<extras::cmex::mxArrayGroup>>& args){

    }
public:
    /// set the persistent arguments
    /// these arguments will be appended to the arguments past using pushTask
    virtual void setPersistentArgs(size_t nrhs, const mxArray* prhs[]){
        /* Expected Syntax
        % setPersistentArgs(Name,Value)
        %
        % Parameters
        % ----------------------------------------------------------------------
        %   'ROIstruct',struct
        %       struct array specifying ROI information
        %       number of elements corresponds with number of particles/windows to track
        %       Fields:
        %           .UUID = '...' char array specifying uniquie identifier for
        %                         ROI
        %           .WindowEdges = [x1,x2,y1,y2] edges of the roi window
        %           .FilterRadius = ## size of radius filter to use for radial
        %                              center
        %           .FilterFactor = ## factor to use when creating logistic
        %                             radius filter. Set to Inf for sharp cutoff
        %           .MinRadius = ## minimum radius (in pixels) to use when
        %                           creating the radial average
        %           .MaxRadius = ## maximum radius (in pixels) to use when
        %                           creating the radial average
        %           .BinWidth = ## (default = 1) bin width (in pixels) for
        %                          radial average
        %           .ReferenceUUID = '...' or {'...','...',...} uuid of
        %                            particles to use as reference
        %           .IsCalibrated = t/f flag if particle has a look-up-table
        %           .Zspline = pp spline describing LUT, used by splineroot
        %           .dZspline = dpp derivative of Zspline, used by splineroot
        % ----------------------------------------------------------------------
        % Global Parameters:
        %   'COMmethod', ''
        %   'DistanceFactor'
        %   'splineroot_TOL'
        %   'splineroot_minStep'
        %   'splineroot_maxItr'
        %   'splineroot_min_dR2frac'
        */

        /* Radial Center Options
        ************************************************************************
        % Input:
        %   I: the image to process
        %   WIND: [N x 4] specifying windows [X1,X2,Y1,Y2], default is entire image
        %   GP (default=5): optional exponent factor to use for magnitude weighting
        %       GP must be either scalar, or numel(GP)==size(WIND,1)
        % Name,Value Parameters:
        % -------------------------
        %   'RadiusFilter',val or [v1,v2,...vN]
        %   'XYc',[X,Y] : particle center estimates
        %   'COMmethod',method
        %       method='meanABS' : use COM on |I-mean(I)| to estimate center for radius filter
        %       method='normal': use COM on unmodified I to estimate center
        %       method='gradmag': use magnitude of image gradient to find COM
        %   'DistanceFactor',value
        %       Rate to use in logistic function defining the filter window around xc,yc
        %       Default value is Inf, which indicates the Hat-function:
        %           W=double(r<Radius) is used instead of a logistic function.
        %       If RadiusFilter==0 DistanceFactor is the exponent of the inverse
        %       distance function use to weight the pixels:
        %           w = w/r^DF
        %       where r is the distance of a pixel from the estimated com or the
        %       specified XYc(n,:) coordinate
        */

        /* imradialavg options
        ************************************************************************
        % Input:
        %   I: the image to process
        %   x0,y0: scalar numbers specifying the coordinates
        %   Rmax(=NaN): scalar specifying maximum radius (NaN indicated image edges
        %       are the limits)
        %   Rmin(=0): minimum radius to use
        %       NOTE: If you specify both Rmax and Rmin you can use the more
        %       logical ordering: imradialavg(__,Rmin,Rmax);
        %   BinWidth(=1): width and spacing of the bins
        */

        /* splineroot options
        ************************************************************************
        % Input:
        %         v: Nx1 array of values to best fit
        %			Note, if all values of v are NaN, the output is NaN
        %			otherwise, dimesions where v(d)==NaN are ignored
        %         pp: N-dimensional spline structure
        %         dpp: (optional) pre-calculated derivative of pp
        %               pass dpp=[] to skip.
        %         TOL: (default=0.001) Algorithm stops when
        %                               (Sum_i(Sp_i(z)-V_mi)^2)<=TOL^2
        %         minStep: (default=20*eps) scalar
        %            if the computed newton step is smaller than minStep the
        %            algorithm returns
        %		  maxItr: (default=10000) maximum number of newton step
        %			 iterations to run
        %		  min_dR2frac: (default = 0.0001) minimim fractional difference
        %		     between succesive iterations.
        %				dR2frac = |R2(i) - R2(i-1)|/R2(i-1)
        %					where R2 is calcualted as (Sum_i(Sp_i(z)-V_mi)^2)
        %			if there is not sufficient change in R2, the algorithm returns
        */

        /* Per ROI Settings
        ------------------------------------------------------------------------
        UUID
        ReferenceUUID
        Window
        MaxRadius for RadialCenter (default=Inf or NAN)
        MinRadius for imradialavg
        MaxRadius for imradialavg
        BinWidth for imradialavg
        IsCalibrated (t/f) for LUT lookup
        Zspline (pp in splineroot, [] if not calibrated)
        dZspline (dpp in splineroot, [] if not calibrated)
        */

        /* Global Settings (applies to all roi)
        ------------------------------------------------------------------------
        COMmethod
        DistanceFactor
        splineroot_TOL
        splineroot_minStep
        splineroot_maxItr
        splineroot_min_dR2frac
        */

        // Require even number of inputs


    }
};
