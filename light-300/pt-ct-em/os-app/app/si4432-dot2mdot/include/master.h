/*
 * master.h
 *
 *  Created on: 2013-12-5
 *      Author: David, zhaoshaowei@yeejoin.com
 */

#ifndef MASTER_H_
#define MASTER_H_



/*!
 * Timings of the demo states.
 *
 * -----------------------------------------------------------------------------
 * |ASSOC|SLAVE1|SLAVE2|SLAVE3|SLAVE4|                  SLEEP                  |
 * -----------------------------------------------------------------------------
 * |200ms|200ms |200ms |200ms |200ms |                  3000ms                 |
 * -----------------------------------------------------------------------------
 * |             1000ms              |                  3000ms                 |
 * -----------------------------------------------------------------------------
 */
//#define STARTUP_TIMEOUT_S                   (3)         // sec
#define TIMEFRAME_ASSOC                     (200)       // msec
#define TIMEFRAME_SU_PER_SLAVE              (200)       // msec
#define TIMEFRAME_SLEEP                     (3)         // sec



#endif /* MASTER_H_ */
