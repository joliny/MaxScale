#!/bin/sh
NARGS=6
TLOG=$1
THOST=$2
TPORT=$3
TMASTER_ID=$4
TUSER=$5
TPWD=$6

if [ $# != $NARGS ] ;
then
echo""
echo "Wrong number of arguments, gave "$#" but "$NARGS" is required"
echo "" 
echo "Usage :" 
echo "        rwsplit.sh <log filename> <host> <port> <master id> <user> <password>"
echo ""
exit 1
fi


RUNCMD=mysql\ --host=$THOST\ -P$TPORT\ -u$TUSER\ -p$TPWD\ --unbuffered=true\ --disable-reconnect\ --silent

TINPUT=test_transaction_routing2.sql
TRETVAL=0
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_transaction_routing2b.sql
TRETVAL=0
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_transaction_routing3.sql
TRETVAL=2
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TMASTER_ID" ]; then 
        echo "$TINPUT FAILED, return value $a when one of the slave IDs was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_transaction_routing3b.sql
TRETVAL=2
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TMASTER_ID" ]; then 
        echo "$TINPUT FAILED, return value $a when one of the slave IDs was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

# test implicit transaction, that is, not started explicitly, autocommit=0
TINPUT=test_transaction_routing4.sql
TRETVAL=0
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_transaction_routing4b.sql
TRETVAL=0
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

# set a var via SELECT INTO @, get data from master, returning server-id: put master server-id value in TRETVAL
TINPUT=select_for_var_set.sql
TRETVAL=$TMASTER_ID

a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit1.sql
TRETVAL=$TMASTER_ID

a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit2.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit3.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit4.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit5.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit6.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_implicit_commit7.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then 
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

TINPUT=test_autocommit_disabled1.sql
TRETVAL=1
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG;
else
        echo "$TINPUT PASSED">>$TLOG ;
fi

TINPUT=test_autocommit_disabled1b.sql
TRETVAL=1
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG;
else
        echo "$TINPUT PASSED">>$TLOG ;
fi

# Disable autocommit in the first session and then test in new session that 
# it is again enabled.
TINPUT=test_autocommit_disabled2.sql
TRETVAL=1
a=`$RUNCMD < ./$TINPUT`
if [ "$a" != "$TRETVAL" ]; then
        echo "$TINPUT FAILED, return value $a when $TRETVAL was expected">>$TLOG;
else
        echo "$TINPUT PASSED">>$TLOG ;
fi

TINPUT=set_autocommit_disabled.sql
`$RUNCMD < ./$TINPUT`
TINPUT=test_after_autocommit_disabled.sql
TRETVAL=$TMASTER_ID
a=`$RUNCMD < ./$TINPUT`
if [ "$a" = "$TRETVAL" ]; then
        echo "$TINPUT FAILED, return value $a when it was not accetable">>$TLOG; 
else 
        echo "$TINPUT PASSED">>$TLOG ; 
fi

