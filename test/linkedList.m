elts = {};
head = 0;
for i = 1:20
  elt.val = i;
  elt.next = head;
  head = head + 1;
  elts{head} = elt;
end
disp(sprintf('Linked list length is now: %d\n', list_len(elts, head)));
disp('Ok we crossed the limit. Go back to just before the list had size 10.');
curr = head;
while (curr > 0)
  disp(elts{curr}.val);
  curr = elts{curr}.next;
end
