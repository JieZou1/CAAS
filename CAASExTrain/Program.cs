using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CAASExTrain
{
    class Program
    {
        static void Main(string[] args)
        {
            //CLR4Tx.CropAndNormalizeObjects();

            //CLR4Tx.NegativeTargetPatches();
            //CLR4Tx.NegativeIsolatorPatches();
            //CLR4Tx.NegativeBasePatches();

            //CLR4Tx.TrainTargetDetection();
            //CLR4Tx.TrainIsolatorDetection();
            //CLR4Tx.TrainBaseDetection();
            
            //CLR4Tx.ToSingleVector();

            //PSM4Tx.CropAndNormalizeObjects();

            //PSM4Tx.NegativeAperturePatches();
            //PSM4Tx.TrainApertureDetection();

            PSM4Tx.ToSingleVector();
        }
    }
}
