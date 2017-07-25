var new_tags = [];
var edit_tags = [];
var delete_id = '';
var edit_id = '';
var publish_id = '';

function new_refresh_tags() {
    var html = '';
    for (var i = 0; i < new_tags.length; ++i) {
        html += '<span class="tag label label-default">' + new_tags[i] +
                '<span class="glyphicon glyphicon-remove remove-tag" aria-hidden="true" data-tag="' +
                encodeURI(new_tags[i]) + '"></span></span>';
    }
    $('#new-view-tags').html(html);
    $('.remove-tag').click(new_remove_tag);
}

function new_remove_tag() {
    var tag = decodeURI($(this).attr('data-tag'));
    for (var i = 0; i < new_tags.length; ++i) {
        if (new_tags[i] === tag) {
            new_tags.splice(i, 1);
        }
    }
    new_refresh_tags();
}

function new_add_tag() {
    var new_tag = $('#new-tag').val();
    if (new_tag == '') {
        return;
    }
    for (var i = 0; i < new_tags.length; ++i) {
        if (new_tags[i] === new_tag) {
            return;
        }
    }
    new_tags.push(new_tag);
    new_refresh_tags();
}

function new_submit() {
    if ($('#new-title').val() == '') {
        $('#new-title-group').addClass('has-error');
        return;
    }
    $.post('/admin/draft/new/', JSON.stringify({
        'title': $('#new-title').val(),
        'content': $('#new-content').summernote('code'),
        'tags': new_tags
    }), function(data, status, xhr) {
        window.location.href = '/admin/draft/';
        return;
    }, 'text');
}

function edit_refresh_tags() {
    var html = '';
    for (var i = 0; i < edit_tags.length; ++i) {
        html += '<span class="tag label label-default">' + edit_tags[i] +
                '<span class="glyphicon glyphicon-remove remove-tag" aria-hidden="true" data-tag="' +
                encodeURI(edit_tags[i]) + '"></span></span>';
    }
    $('#edit-view-tags').html(html);
    $('.remove-tag').click(edit_remove_tag);
}

function edit_remove_tag() {
    var tag = decodeURI($(this).attr('data-tag'));
    for (var i = 0; i < edit_tags.length; ++i) {
        if (edit_tags[i] === tag) {
            edit_tags.splice(i, 1);
        }
    }
    edit_refresh_tags();
}

function edit_add_tag() {
    var new_tag = $('#edit-tag').val();
    if (new_tag == '') {
        return;
    }
    for (var i = 0; i < edit_tags.length; ++i) {
        if (edit_tags[i] === new_tag) {
            return;
        }
    }
    edit_tags.push(new_tag);
    edit_refresh_tags();
}

function edit_submit() {
    if ($('#edit-title').val() == '') {
        $('#edit-title-group').addClass('has-error');
        return;
    }
    $.post('/admin/draft/edit/', JSON.stringify({
        'id': edit_id,
        'title': $('#edit-title').val(),
        'content': $('#edit-content').summernote('code'),
        'tags': edit_tags
    }), function(data, status, xhr) {
        window.location.href = '/admin/draft/';
        return;
    }, 'text');
}

function delete_draft() {
    $.get('/admin/draft/delete/' + delete_id + '/', function(data, status) {
        if (data != 'ok') {
            $('#alert-failure').css('display', 'block');
        } else {
            location.reload(true);
        }
    });
}

function edit_show() {
    var id_encoded = $(this).attr('data-id');
    var id = decodeURI(id_encoded);
    $.get('/admin/draft/json/' + id_encoded + '/', function(data, status) {
        $('#edit-title').val(data.title);
        edit_tags = data.tags;
        edit_refresh_tags();
        $('#edit-content').summernote('code', data.content);
    });
    $('#modal-edit-draft').modal('show');
}

function publish() {
    $.post('/admin/draft/publish/', JSON.stringify({
        'id': publish_id,
        'now': $('input[type=\'radio\']:checked').val() == 'now'
    }), function(data, status, xhr) {
        window.location.href = data;
    }, 'text');
}

$(document).ready(function() {
    $('#new-add-tag').click(new_add_tag);
    $('#new-submit').click(new_submit);
    $('#edit-add-tag').click(edit_add_tag);
    $('#edit-submit').click(edit_submit);
    $('.delete-draft').click(function() {
        delete_id = decodeURI($(this).attr('data-id'));
        $('#alert-failure').css('display', 'none');
        $('#modal-delete').modal('show');
    });
    $('.edit-draft').click(edit_show);
    $('.publish').click(function() {
        publish_id = decodeURI($(this).attr('data-id'));
        $('#modal-publish').modal('show');
    });
    $('#new-content').summernote({
        height: 300
    });
    $('#edit-content').summernote({
        height: 300
    });
    $('#delete-draft').click(delete_draft);
    $('#publish').click(publish);
});