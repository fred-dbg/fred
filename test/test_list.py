#!/usr/bin/python

class list_el:
    def __init__(self, val=None, next=None):
        self.val = val
        self.next = next
    def __str__(self):
        return str(self.val)

def list_len(elt):
    count = 0
    while elt != None:
        elt = elt.next
        count += 1

    return count

if __name__ == "__main__":
    head = None
    print "Initial Linked list length: %d" % list_len(head)

    for i in range(1, 21):
        curr = list_el(i, head)
        head = curr

    print "Linked list length is now: %d" % list_len(head)
    print "Ok we crossed the limit.  Let's go back to just before the list had size 10."

    curr = head
    while curr != None:
        print curr.val
        curr = curr.next

