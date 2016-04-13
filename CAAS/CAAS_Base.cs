using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Emgu.CV;

namespace CAAS
{
    public abstract class CAAS_Base
    {
        protected Mat colorImage;   //The input Color image to be processed.
        protected Mat grayImage;    //The gray scale image

        protected CAAS_Result result;   //The detection result

        /// <summary>
        /// Do the general initialization work
        /// </summary>
        /// <param name="color_image"></param>
        protected CAAS_Base(Mat color_image)
        {
            colorImage = color_image;
            grayImage = new Mat(); CvInvoke.CvtColor(color_image, grayImage, Emgu.CV.CvEnum.ColorConversion.Bgr2Gray);

            result = new CAAS_Result();
        }

        protected void LineSegmentDetection()
        { 
        }
    }
}
