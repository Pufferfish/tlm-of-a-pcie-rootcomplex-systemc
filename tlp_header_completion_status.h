//
//  tlp_header_completion_status.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__tlp_header_completion_status__
#define __Even__tlp_header_completion_status__

enum CompletionStatus {
	SUCCESSFUL_COMPLETION = 0,
	UNSUPPORTED_REQUEST,
	CONFIG_REQ_RETRY_STATUS,
	COMPLETION_ABORT
};

#endif
