c	$Id$	
c----------------------------------------------------------------------
c       
c       This file contains the following routines
c       
c       givfld
c       givrng
c       present
c       setcat
c       setdpr
c       setpsn
c       setray
c
c       accver
c       mountd
c       
c       
c       
c       
c       
c       
c----------------------------------------------------------------------
        subroutine givfld
     1          (kbuf,name,r1,r2,m,vector,lenout,rzero,gs,badval)
c       
c       return the values for the "name" field converted to scientific units
c       between the ranges "r1" and "r2".
c       
        integer kbuf(*), dap_ndx_name
        dimension vector(*)
        logical same
        character*2 name, id, kflg
c       
c       
        if( r1 .gt. r2 ) then
            call typstg( 6, 'r1 is greater than r2 ^', 1, 0 )
        endif
        lenout=0
c       determine the source id of the "name" field
        idn = dap_ndx_name( name )
        if( idn .lt. 0 ) then
            go to 90
        endif
c       
c       range to first gate and gate spacing (ask for 2 gates of range info)
        call dap_rng_info( 1, 2, vector, ngts )
        r0x = vector(1)
        gs = vector(2) - r0x
c       
c       exit if zero gate spacing for no data
        if(gs.eq.0..or.ngts.eq.0)then
            call typstg( 6, id, 1, 2 )
            call typstg( 6, 'data is garbage ^', 1, 0 )
            go to 90
        endif
c       
        ig1=max0(1,int((r1-r0x)/gs+2.))
        ig2=min0(ngts,int((r2-r0x)/gs+1.))
c       range to the first gate returned
        rzero=r0x+(ig1-1)*gs
        lenx = ig2-ig1+1
c
c   for now since there can be variable gate spacing just
c   return all the gates
    	rzero = vector(1)
    	lenx = ngts

        if( m .lt. 1 ) then
c       assume this is just a request for range and gate info
            lenout = lenx
            go to 90
        endif
c       no. gates to unpack
        lenout = min0(lenx,m)
c       
        call dap_givfld( idn, ig1, lenout, vector, badval )
c
        do 33 igt = lenout+1, m
            vector(igt) = badval
  33    continue
c       
c       
  90    continue
        return
        end
c----------------------------------------------------------------------
        subroutine givgts(kbuf,name,ig1,m,vector,lenout,badval)
c       
c       return the values for the "name" field converted to scientific units
c       between the gates ig1, ig2
c       
        integer kbuf(*), dap_ndx_name
        dimension vector(*)
        logical same
        character*2 name, id, kflg
c       
c       
        lenout=0
c       determine the source id of the "name" field
        idn = dap_ndx_name( name )
        if( idn .lt. 0 ) then
            go to 90
        endif
c       
c       no. gates to unpack
    	call dap_rng_info(1,1,x,ng)
c
    	if((ig1-1) +m .gt. ng ) then
            lenout = ng -(ig1-1)
    	else
    	    lenout = m
    	endif
c       
        call dap_givfld( idn, ig1, lenout, vector, badval )
c
        do 33 igt = lenout+1, m
            vector(igt) = badval
  33    continue
c       
  90    continue
        return
        end
c----------------------------------------------------------------------
        subroutine givrng
     1          (kbuf, name, r1, r2, m, vector, lenout
     +	    	, rzero, gs, changed, ig1 )
c
c   the purpose of givrng is to return the number of the first gate
c   and the number of gates following to cover the ranges "r1" and "r2";
c   (this info can be used with "givgts")
c   to fill vector with the range values corresponding to these
c   particular gates;
c   and to return a flag indicating that the range information will
c   be different from the information passed in in vector
c
c   It is done this way to accommodate variable gate spacing
c       
        integer kbuf(*)
        dimension vector(*)
    	logical changed
    	dimension rg(2)
c       
c       
    	call dap_range_gate( r1, ig1, rr1 )
    	call dap_range_gate( r2, ig2, rr2 )
    	lenout = ig2 -ig1 +1

        call dap_rng_info( ig1, 2, rg, ngts )
    	rzero = rg(1)
        gs = rg(2) - rg(1)


    	lenout = min0( m, lenout )
        call dap_rng_info((ig1-1)+lenout, 1, rr2, ngts )
    	changed = vector(1) .ne. rr1 .or. vector(lenout) .ne. rr2
c       
    	if(changed .and. lenout .gt. 0) then
            call dap_rng_info( ig1, lenout, vector, ngts )
    	endif
c
  90    continue
        return
        end
c---------------------------------------------------------------------
        logical function present( kbuf, mne )
c   this routine determines if the source field for this mnemonic is present
c   in the input data.
c
        implicit integer( a-z )
        dimension   kbuf(*)
        character   mne*(*)
        character*2 id
c
c
        i = dap_parm_present( mne )
        if( i .eq. -1 ) then
            present = .false.
        else
            present = .true.
        endif
c
        return
        end
c---------------------------------------------------------------------
        logical function prznt( kbuf, mne )
c   this routine determines if the source field for this mnemonic is present
c   in the input data.
c
        implicit integer( a-z )
        dimension   kbuf(*)
        character   mne*(*)
        character*2 id
c
c
        i = dap_parm_present( mne )
        if( i .eq. -1 ) then
            prznt = .false.
        else
            prznt = .true.
        endif
c
        return
        end
c----------------------------------------------------------------------
      subroutine setcat
     1  ( buffer, lenbf, lundx, lucat, ludat, uics, lens, ierr )
c
c  this routine scans an input control string for attributes
c  that specify which tapes to mount and where to start and stop
c  reading data.
c  if all necessary information is found, then on return from this
c  routine the first useable record should be in the record buffer.
c
        implicit        integer( a-z )
        dimension       buffer(*)
        character       uics*(*)
c
        common  / ufapu /       ncs
     a,         taptim( 6 ),    ccstm1( 6 ),    ccstm2( 6 )   
     a,         lenbuf,         lundat,         nfiles
     a,         intpt,          intno,          ivol,           iscan
     a,         irec,           krec,           ueof
c
        common / ufapc /        ufcs
        character*2222 ufcs
        integer taptim, ccstm1, ccstm2
c
c
    	character*77	str
        logical         first
        data            first   / .true. /
c
c
    	ierr = 0
c   print out the access version
        call accver()
        nfiles = 0
        ivol = 0
        iscan = 0
        lenbuf = lenbf
    	ix = namefx( 'REC_LEN', i )
    	call putfx( i, nwds16(lenbuf))
        lundat = ludat
c
        if( lens .lt. 1 ) then
          call typstg( 6, 'no ufap input control string  ^', 1, -1 )
          ierr = 999
          go to 90
        endif
c
c  copy and compress the input control string
        ncs = 1
        ufcs(1:1) = ';'
        do 11 i = 1, lens
            if( uics(i:i) .ne. ' ' ) then
                ncs = ncs+1
                ufcs(ncs:ncs) = uics(i:i)
            endif
 11     continue
        ncs = ncs+1
        ufcs(ncs:ncs) = ';'
c
c
        ia = indexa( ufcs, 1, ncs, 4, 'SHOW' )
        if( ia .gt. 0 ) then
            iarg = 1
  22        continue
            call attarg( ufcs, ia, ncs, iarg, ia1, na )
            if( na .gt. 0 ) then
                call copyc( ufcs, ia1, na, str, 1 )
                ix = namefx( str(1:na), i )
                call putfx( i, 1 )
                iarg = iarg+1
                go to 22
            endif
        endif
c
c  extract the start and stop times
        call cdatmz( ufcs, ncs, ccstm1, ccstm2, ierr )
        if( ierr .ne. 0 ) then
          go to 90
        endif
c
c  get set to access the data and read the first record
        call mountd( ierr )
        if( ierr .ne. 0 ) then
          go to 90
        endif
c
c  read in the first record
c        call setray( buffer, 1, ierr )
        if( ierr  .ne. 0 ) then
          go to 90
        endif
c
c
 90     continue
        return
        end
c----------------------------------------------------------------------
        subroutine setray (kbuf, kdir, ierr)
c       
c       move to the next ray
c       ignore kdir --- always go forward
c       
        implicit        integer( a-z )
    	parameter( END_OF_TIME_SPAN=-9 )
    	parameter( OK_RAY=0 )
    	real x, y, uazim, uelev, ufixed
        dimension       kbuf(*)
c       
        common  / ufapu /       ncs
     a,         taptim( 6 ),    ccstm1( 6 ),    ccstm2( 6 )   
     a,         lenbuf,         lundat,         nfiles
     a,         intpt,          intno,          ivol,           iscan
     a,         irec,           krec,           ueof
c       
        common / ufapc /        ufcs
        character*2222 ufcs
        integer taptim, ccstm1, ccstm2
        logical ueof
c       
        logical nxrset
        dimension       dt( 6 )
c       
c       
        ierr = 0
        kount=0
        ueof = .false.
c       
  11    continue
    	state = dap_next_ray( lundat, kbuf )
c       
    	if(state .eq. END_OF_TIME_SPAN ) then
    	    ierr = 999
    	    go to 90
    	endif
c
        iscan = kswepn( kbuf )
        ivol = kvoln( kbuf )
        irec = kprnv( kbuf )
c       
  90    continue
        return
        end
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
C           Unix DEPENDENT CODE
c-----------------------------------------------------------------------
    	subroutine accver()
c   dump out the current access version date
        call dmplin( 'qDAP unix VERSION Novermber 1997' , 0 )
    	return
    	end
c-----------------------------------------------------------------------
    	subroutine mountd ( ierr ) 
c   make sure that the tape with name as identifier is mounted 
c   on unit lun 
c
        implicit integer( a-z )
        common  / ufapu /       ncs
     a,         taptim( 6 ),    ccstm1( 6 ),    ccstm2( 6 )   
     a,         lenbuf,         lundat,         nfiles
     a,         intpt,          intno,          ivol,           iscan
     a,         irec,           krec,           ueof
c
        common / ufapc /        ufcs
        character*2222 ufcs
c
        character*88 text
        logical ueof
c
    	integer control_string(1000), radar_name(4)
    	integer directory(40)
    	integer input_file(40)
    	integer tm1, tm2
c
c
    	ierr = 0
c
c   convert times to unix time
    	call time2unix( ccstm1, tm1 )    	
    	call time2unix( ccstm2, tm2 )    	
c 
        in = indexa( ufcs, 1, ncs, 4, 'INSTRUMENT' )
        if( in .gt. 0 ) then
            call attarg( ufcs, in, ncs, 1, i1, ni )
        endif

    	if( in .gt.0 .and. ni .gt. 0 ) then    
    	    call copycn( ufcs, i1, ni, radar_name, 1 )
    	else
    	    ni = 4
    	    call copycn( 'NONE', 1, ni, radar_name, 1 )
    	endif
    	call put8( radar_name, ni+1, 0 )
c
c   open the file and read to the first usable ray
c
        in = indexa( ufcs, 1, ncs, 4, 'INPUT' )
        dir = indexa( ufcs, 1, ncs, 4, 'DIRECTORY' )
c
    	if(in .gt. 0 ) then
            call attarg( ufcs, in, ncs, 1, i1, ni )
    	    call copycn( ufcs, i1, ni, input_file, 1 )
    	    call put8( input_file, ni+1, 0 )
    	    ierr = dap_mnt_f(input_file, radar_name, tm1, tm2
     +	    	, ivol, iscan, irec)
    	    if( ierr  .ne. 0 ) then
    	        write(*,*)'Error mountd first read...err: '
     +	            , ierr
                go to 90
            endif
    	elseif(dir .gt. 0 ) then
            call attarg( ufcs, dir, ncs, 1, i1, ni )
    	    if( ni .gt. 0 ) then
    	    	call copycn( ufcs, i1, ni, directory, 1 )
    	    	call put8( directory, ni+1, 0 )
    	    	ierr = dap_mnt_s( directory, radar_name
     +	    	    , tm1, tm2, ivol, iscan, irec )
    	    	if(ierr .eq. 0 ) then
c  read in the first record
    	    	    call setray( buffer, 1, ierr )
            	    if( ierr  .ne. 0 ) then
    	    	    	write(*,*)'Error in mountd on first read...err: '
     +	    	    	    , ierr
            	    	go to 90
            	    endif
    	    	endif
    	    else
    	     	ierr = 999
    	    endif
    	else
    	    ierr = 999
    	endif
c
    	if(ierr .ne. 0 ) then
    	    write(*,*) 'Missing directory or input attribute!'
    	    return
    	endif
c
c
        iscan = kswepn( kbuf )
        ivol = kvoln( kbuf )
        irec = kprnv( kbuf )
      	nfiles = 0
      	krec = 0
c
 90   	continue
      	return
      	end
c--------------------------------------------------------------------------
    	subroutine setdpr( flag )
    	logical flag

        if( flag ) then
            call dap_setdpr( 1 )
        else
            call dap_setdpr( 0 )
        endif
        return
        end
c--------------------------------------------------------------------------
    	real function uazim()
c   this bunch of crap is necessary because C returns a double precision
c   for a real function call
    	call cuazim(x)
    	uazim = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function udmrc()
    	call cudmrc(x)
    	udmrc = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function udmprf(buf,name)
    	call cudmprf(name, x)
    	udmprf = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function uelev()
    	call cuelev(x)
    	uelev = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function ufixed()
    	call cufixed(x)
    	ufixed = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function ugealt()
    	call cugealt(x)
    	ugealt = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function uhbwid()
    	call cuhbwid(x)
    	uhbwid = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function uhight()
    	call cuhight(x)
    	uhight = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function ulatit()
    	call culatit(x)
    	ulatit = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function ulongt()
    	call culongt(x)
    	ulongt = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function urotat()
    	call curotat(x)
    	urotat = x
    	return
    	end
c--------------------------------------------------------------------------
    	real function uvenyq()
    	call cuvenyq(x)
    	uvenyq = x
    	return
    	end
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
c--------------------------------------------------------------------------
