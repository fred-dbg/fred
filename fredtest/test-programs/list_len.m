function length = list_len(elts, head)
  length = 1;
  while (elts{head}.next ~= 0)
    head = elts{head}.next;
    length = length + 1;
  end
end
