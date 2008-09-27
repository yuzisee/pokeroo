/***************************************************************************
 *   Copyright (C) 2005-2007 by Joseph Huang                               *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* Dev Testing Flags */
#undef NO_LOG_FILES
#define DEBUGASSERT

/***********************/

/* Game Flags */
#undef EXTERNAL_DEALER

/******************/

#ifndef NO_LOG_FILES
/* Mode Flags (Depends on NO_LOG_FILES) */
#define LOGPOSITION
#undef WINRELEASE
/**********************************/
#undef DEBUG_GAIN

#define GRAPHMONEY "chipcount.csv"
#define REPRODUCIBLE

/* Savegame debugging */
#define DEBUG_SINGLE_HAND
#endif




/* AI Processing Interface */

#undef PROGRESSUPDATE
#undef SUPERPROGRESSUPDATE
#undef DEBUG_TESTDEALINTERFACE

/***********************/

/* Randomizer Flags */

#undef FORCESEED
#undef COOLSEEDINGVIEWER

/*****************/



/* functionbase.h Debugging */
#undef DEBUG_TRACE_ZERO
#undef DEBUG_TRACE_SEARCH


