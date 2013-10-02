{% extends 'common_group.cpp' %}

{% block maincode %}
    // USES_VARIABLES { _synaptic_pre, _synaptic_post, _post_synaptic,
    //                  _pre_synaptic, _num_source_neurons, _num_target_neurons,
    //                  rand, _source_offset, _target_offset}
	srand((unsigned int)time(NULL));
	int _buffer_size = 1024;
	int *_prebuf = new int[_buffer_size];
	int *_postbuf = new int[_buffer_size];
	int *_synprebuf = new int[1];
	int *_synpostbuf = new int[1];
	int _curbuf = 0;
	int _synapse_idx = _synaptic_pre_object.attr("shape")[0];
	for(int i=0; i<_num_source_neurons; i++)
	{
		for(int j=0; j<_num_target_neurons; j++)
		{
		    const int _vectorisation_idx = j;
		    const int _presynaptic_idx = i + _source_offset;
		    const int _postsynaptic_idx = j + _target_offset;
			// Define the condition
		    {{ super() }}
			// Add to buffer
			if(_cond)
			{
			    if (_p != 1.0) {
			        // We have to use _rand instead of rand to use our rand
			        // function, not the one from the C standard library
			        if (_rand(_vectorisation_idx) >= _p)
			            continue;
			    }

			    for (int _repetition=0; _repetition<_n; _repetition++) {
                    _prebuf[_curbuf] = _presynaptic_idx - _source_offset;
                    _postbuf[_curbuf] = _postsynaptic_idx - _target_offset;
                    _curbuf++;
                    // Flush buffer
                    if(_curbuf==_buffer_size)
                    {
                        _flush_buffer(_prebuf, _synaptic_pre_object, _curbuf);
                        _flush_buffer(_postbuf, _synaptic_post_object, _curbuf);
                        _curbuf = 0;
                    }
                    // Directly add the synapse numbers to the neuron->synapses
                    // mapping
                    _synprebuf[0] = _synapse_idx;
                    _synpostbuf[0] = _synapse_idx;
                    py::object _pre_synapses = (py::object)PyList_GetItem(_pre_synaptic, _presynaptic_idx - _source_offset);
                    py::object _post_synapses = (py::object)PyList_GetItem(_post_synaptic, _postsynaptic_idx - _target_offset);
                    _flush_buffer(_synprebuf, _pre_synapses, 1);
                    _flush_buffer(_synpostbuf, _post_synapses, 1);
                    _synapse_idx++;
                }
			}
		}

	}
	// Final buffer flush
	_flush_buffer(_prebuf, _synaptic_pre_object, _curbuf);
	_flush_buffer(_postbuf, _synaptic_post_object, _curbuf);
	delete [] _prebuf;
	delete [] _postbuf;
	delete [] _synprebuf;
	delete [] _synpostbuf;
{% endblock %}

{% block support_code_block %}
// Flush a buffered segment into a dynamic array
void _flush_buffer(int *buf, py::object &dynarr, int N)
{
	int _curlen = dynarr.attr("shape")[0];
	int _newlen = _curlen+N;
	// Resize the array
	py::tuple _newlen_tuple(1);
	_newlen_tuple[0] = _newlen;
	dynarr.mcall("resize", _newlen_tuple);
	// Get the potentially newly created underlying data arrays
	int *data = (int*)(((PyArrayObject*)(PyObject*)dynarr.attr("data"))->data);
	// Copy the values across
	for(int i=0; i<N; i++)
	{
		data[_curlen+i] = buf[i];
	}
}

{{ super() }}

{% endblock %}
