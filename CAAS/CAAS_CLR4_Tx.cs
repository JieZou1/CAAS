using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Emgu.CV;

namespace CAAS
{
    public sealed class CAAS_CLR4_Tx : CAAS_Base
    {
        public static CAAS_Result Detect(Mat image)
        {
            CAAS_CLR4_Tx tx = new CAAS_CLR4_Tx(image);
            tx.Detect();
            return tx.result;
        }

        CAAS_CLR4_Tx(Mat color_image) : base(color_image)
        { 
        }

        private void Detect()
        {
        }
    }
}
