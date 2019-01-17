# Format Strings for Sitemaps
Use standard java format strings. See more info for dates.
[https://dzone.com/articles/java-string-format-examples](https://dzone.com/articles/java-string-format-examples)

# Node 2 Debugging
Stopped running again, and having problems with the processor seemingly
locking up. Does not send messages or continue with loops, or at least doesn't
trigger periodic messages.

## Debugging Info
 - Stopped on 2019-01-02 22:38:57 sometime after sending motion message
    - May have be stuck in a loop waiting for the temperature sensor
    - May have had power glitch
    - Timer wrap around problems less likely since all are using the same
      software.
    - Gateway node was reset 11 seconds later after receiving no messages
      from any node in the last 10 seconds, reset again after no messages
      for another 10 seconds, and reset again after no messages for 10
      seconds. Received last message from node 4 at 22:38:58 and received
      next message from node 4 at 22:39:30
    - Node 2 rssi at -43 before dropping out
    - Node 4 rssi at -28 and node 3 at -37 before reset. Node 4 at -43 and
      Node 3 at -42 after reset
    - Reset around 1 am 2019-01-04
 - Messages stopped on 2019-01-04 11:36:15
    - Messages still being sent over serial to the laptop, but Acks not
      received from the gateway. Still attempting to send messages, but
      not receiving Acks? 
 - Power cycled on 2019-01-04 15:46:xx
    - Previously tx led had stopped blinking
