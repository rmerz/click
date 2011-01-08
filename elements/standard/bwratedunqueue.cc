// -*- c-basic-offset: 4 -*-
/*
 * ratedunqueue.{cc,hh} -- element pulls as many packets as possible from
 * its input, pushes them out its output
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2010 Meraki, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "bwratedunqueue.hh"
CLICK_DECLS

BandwidthRatedUnqueue::BandwidthRatedUnqueue()
{
}

BandwidthRatedUnqueue::~BandwidthRatedUnqueue()
{
}

bool
BandwidthRatedUnqueue::run_task(Task *)
{
    bool worked = false;
    _runs++;
    _rate.refill();

    if (!_active)
	return false;

    if (_rate.contains(RATE_FILL_MIN)) {
	if (Packet *p = input(0).pull()) {
	    _rate.remove(p->length());
	    _pushes++;
	    worked = true;
	    output(0).push(p);
	} else {
	    _failed_pulls++;
	    if (!_signal)
		return false;	// without rescheduling
	}
    } else {
	_timer.schedule_after(Timestamp::make_jiffies(_rate.epochs_until_contains(RATE_FILL_MIN)));
	_empty_runs++;
	return false;
    }
    _task.fast_reschedule();
    if (!worked)
	_empty_runs++;
    return worked;
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(RatedUnqueue)
EXPORT_ELEMENT(BandwidthRatedUnqueue)
ELEMENT_MT_SAFE(BandwidthRatedUnqueue)
