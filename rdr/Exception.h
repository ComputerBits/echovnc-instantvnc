//
// Copyright (C) 2002 RealVNC Ltd.  All Rights Reserved.
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
// USA.

#ifndef __RDR_EXCEPTION_H__
#define __RDR_EXCEPTION_H__

#include <stdio.h>
#include <string.h>

namespace rdr {

  struct Exception {
    enum { len = 256 };
    char str_[len];
	char error_[len];
    Exception(const char* s=0, const char* e="rdr::Exception") {
      str_[0] = 0;
      strncat_s(str_, len, e, len-1);
      if (s) {
        strncat_s(str_, len, ": ", len-1-strlen(str_));
        strncat_s(str_, len, s, len-1-strlen(str_));
      }
    }
    virtual const char* str() const { return str_; }
  };

  struct SystemException : public Exception {
    int err;
    SystemException(const char* s, int err_) : err(err_) {
      str_[0] = 0;
	  error_[0] = 0;
      strncat_s(str_, len, "rdr::SystemException: ", len-1);
      strncat_s(str_, len, s, len-1-strlen(str_));
      strncat_s(str_, len, ": ", len-1-strlen(str_));
	  strerror_s(error_, len, err);
      strncat_s(str_, len, error_, len-1-strlen(str_));
      strncat_s(str_, len, " (", len-1-strlen(str_));
      char buf[20];
      sprintf_s(buf, 20,"%d",err);
      strncat_s(str_, len, buf, len-1-strlen(str_));
      strncat_s(str_, len, ")", len-1-strlen(str_));
    }
  }; 

  struct TimedOut : public Exception {
    TimedOut(const char* s=0) : Exception(s,"rdr::TimedOut") {}
  };
 
  struct EndOfStream : public Exception {
    EndOfStream(const char* s=0) : Exception(s,"rdr::EndOfStream") {}
  };

  struct FrameException : public Exception {
    FrameException(const char* s=0) : Exception(s,"rdr::FrameException") {}
  };

}

#endif
