#include "predictor.h"

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////

#define bimodal false
#define global true
#define gshare false
#define correle false
#define local false
#define mix false


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Constructeur du prédicteur
PREDICTOR::PREDICTOR(char *prog, int argc, char *argv[])
{
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> pcbits countbits\n", prog);
      exit(-1);
   }


   #if binomal

      uint32_t pcbits    = strtoul(argv[0], NULL, 0);
      uint32_t countbits = strtoul(argv[1], NULL, 0);

      nentries = (1 << pcbits);        // nombre d'entrées dans la table
      pcmask   = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
      countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
      table    = new uint32_t[nentries]();

   #endif

///////////////////////////////////////////

   #if global

      uint32_t pcbits    = strtoul(argv[0], NULL, 0);
      uint32_t countbits = strtoul(argv[1], NULL, 0);


      nentries = (1 << pcbits);        // nombre d'entrées dans la table
      countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
      table    = new uint32_t[nentries]();
		historic = 0;
      mask_hist = (nentries - 1);

   #endif

	 ///////////////////////////////////////////

	#if gshare

	   uint32_t pcbits    = strtoul(argv[0], NULL, 0);
	   uint32_t countbits = strtoul(argv[1], NULL, 0);


	   nentries = (1 << pcbits);        // nombre d'entrées dans la table
	   countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
	   table    = new uint32_t[nentries]();
		historic = 0;
	   mask = (nentries - 1);

	#endif

}

/////////////////////////////////////////////////////////////
//////////////////////// BIMODAL ////////////////////////////

#if bimodal

bool PREDICTOR::GetPrediction(UINT64 PC)
{
   uint32_t v = table[PC & pcmask];
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t v = table[PC & pcmask];
   if( (v == (countmax+1)/2) && (resolveDir == NOT_TAKEN) ) table[PC & pcmask] = 0;
   else if( (v == (countmax-1)/2) && (resolveDir == TAKEN) ) table[PC & pcmask] = countmax;
   else table[PC & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
}

#endif

/////////////////////////////////////////////////////////////
/////////////////////// GLOBAL //////////////////////////////

#if global

bool PREDICTOR::GetPrediction(UINT64 PC)
{
   uint32_t v = table[historic & mask_hist];
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t v = table[historic & mask_hist];
   historic = (mask_hist) & ((resolveDir == TAKEN) ? (historic << 1)+1 : (historic << 1));

   if( (v == (countmax+1)/2) && (resolveDir == NOT_TAKEN) ) table[historic & mask_hist] = 0;
   else if( (v == (countmax-1)/2) && (resolveDir == TAKEN) ) table[historic & mask_hist] = countmax;
   else table[historic & mask_hist] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);

   // cout << "Historique : " << historic & mask_hist << endl;
}

#endif

/////////////////////////////////////////////////////////////
/////////////////////// GSHARE //////////////////////////////

#if gshare

bool PREDICTOR::GetPrediction(UINT64 PC)
{
   uint32_t v = table[(PC ^ historic) & mask];
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t v = table[(PC ^ historic) & mask];
   historic = (mask) & ((resolveDir == TAKEN) ? (historic << 1)+1 : (historic << 1));

   if( (v == (countmax+1)/2) && (resolveDir == NOT_TAKEN) ) table[(PC ^ historic) & mask] = 0;
   else if( (v == (countmax-1)/2) && (resolveDir == TAKEN) ) table[(PC ^ historic) & mask] = countmax;
   else table[(PC ^ historic) & mask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);

   // cout << "Historique : " << historic & mask_hist << endl;
}

#endif

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
   // This function is called for instructions which are not
   // conditional branches, just in case someone decides to design
   // a predictor that uses information from such instructions.
   // We expect most contestants to leave this function untouched.
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
